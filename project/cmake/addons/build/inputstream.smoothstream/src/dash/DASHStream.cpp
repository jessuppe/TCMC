/*
* DashStream.cpp
*****************************************************************************
* Copyright(C) 2015, liberty_developer
*
* Email: liberty.developer@xmail.net
*
* This source code and its use and distribution, is subject to the terms
* and conditions of the applicable license agreement.
*****************************************************************************/

#include "DASHStream.h"

#include <iostream>
#include <cstring>
#include "../oscompat.h"
#include <math.h>

using namespace dash;

DASHStream::DASHStream(DASHTree &tree, DASHTree::StreamType type)
  :tree_(tree)
  , type_(type)
  , observer_(0)
  , current_period_(tree_.periods_.empty() ? 0 : tree_.periods_[0])
  , current_adp_(0)
  , current_rep_(0)
{
}

bool DASHStream::download_segment()
{
  segment_buffer_.clear();
  absolute_position_ = 0;
  segment_read_pos_ = 0;

  char rangebuf[128];
  if (!current_seg_ || !~current_seg_->range_end_)
    return false;

  std::string strURL(current_rep_->url_);
  sprintf(rangebuf, "%" PRIu64, tree_.minLiveTime_ + current_seg_->range_end_);
  strURL.replace(strURL.find("{start time}"), 12, rangebuf);

  return download(strURL.c_str());
}

bool DASHStream::write_data(const void *buffer, size_t buffer_size)
{
  segment_buffer_ += std::string((const char *)buffer, buffer_size);
  return true;
}

bool DASHStream::prepare_stream(const DASHTree::AdaptationSet *adp,
  const uint32_t width, const uint32_t height,
  uint32_t min_bandwidth, uint32_t max_bandwidth)
{
  width_ = type_ == DASHTree::VIDEO ? width : 0;
  height_ = type_ == DASHTree::VIDEO ? height : 0;

  uint32_t avg_bandwidth = tree_.bandwidth_;

  bandwidth_ = min_bandwidth;
  if (avg_bandwidth > bandwidth_)
    bandwidth_ = avg_bandwidth;
  if (max_bandwidth && bandwidth_ > max_bandwidth)
    bandwidth_ = max_bandwidth;

  stopped_ = false;

  bandwidth_ = static_cast<uint32_t>(bandwidth_ *(type_ == DASHTree::VIDEO ? 0.9 : 0.1));

  current_adp_ = adp;

  return select_stream(false, true);
}

bool DASHStream::start_stream(const uint32_t seg_offset)
{
  segment_buffer_.clear();
  current_seg_ = current_rep_->get_segment(seg_offset);
  if (!current_seg_ || !current_rep_->get_next_segment(current_seg_))
    stopped_ = true;
  absolute_position_ = current_rep_->get_next_segment(current_seg_)->range_begin_;
  stopped_ = false;
  return true;
}

uint32_t DASHStream::read(void* buffer, uint32_t  bytesToRead)
{
  if (stopped_)
    return 0;

  if (segment_read_pos_ >= segment_buffer_.size())
  {
    current_seg_ = current_rep_->get_next_segment(current_seg_);
    if (!download_segment() || segment_buffer_.empty())
    {
      stopped_ = true;
      return 0;
    }
  }
  if (bytesToRead)
  {
    uint32_t avail = segment_buffer_.size() - segment_read_pos_;
    if (avail > bytesToRead)
      avail = bytesToRead;
    memcpy(buffer, segment_buffer_.data() + segment_read_pos_, avail);

    segment_read_pos_ += avail;
    absolute_position_ += avail;
    return avail;
  }
  return 0;
}

bool DASHStream::seek(uint64_t const pos)
{
  // we seek only in the current segment
  if (pos >= absolute_position_ - segment_read_pos_)
  {
    segment_read_pos_ = static_cast<uint32_t>(pos - (absolute_position_ - segment_read_pos_));
    if (segment_read_pos_ > segment_buffer_.size())
    {
      segment_read_pos_ = static_cast<uint32_t>(segment_buffer_.size());
      return false;
    }
    absolute_position_ = pos;
    return true;
  }
  return false;
}

bool DASHStream::seek_time(double seek_seconds, double current_seconds, bool &needReset)
{
  if (!current_rep_)
    return false;

  uint32_t choosen_seg(~0);

  if (!current_adp_->segment_durations_.data.empty())
  {
    uint64_t sec_in_ts = static_cast<uint64_t>(seek_seconds * current_adp_->timescale_);
    choosen_seg = 0;
    while (choosen_seg < current_adp_->segment_durations_.data.size() && sec_in_ts > *current_adp_->segment_durations_[choosen_seg])
      sec_in_ts -= *current_adp_->segment_durations_[choosen_seg++];
  }
  else if (current_rep_->duration_ > 0 && current_rep_->timescale_ > 0)
  {
    uint64_t sec_in_ts = static_cast<uint64_t>(seek_seconds * current_rep_->timescale_);
    choosen_seg = static_cast<uint32_t>(sec_in_ts / current_rep_->duration_);
  }
  const DASHTree::Segment* old_seg(current_seg_);
  if ((current_seg_ = current_rep_->get_segment(choosen_seg)))
  {
    needReset = true;
    if (current_seg_ != old_seg)
      download_segment();
    else if (seek_seconds < current_seconds)
    {
      absolute_position_ -= segment_read_pos_;
      segment_read_pos_ = 0;
    }
    else
      needReset = false;
    return true;
  }
  else
    current_seg_ = old_seg;
  return false;
}

bool DASHStream::select_stream(bool force, bool justInit)
{
  const DASHTree::Representation *new_rep(0), *min_rep(0);

  if (force && absolute_position_ == 0) //already selected
    return true;

  unsigned int bestScore(~0);

  for (std::vector<DASHTree::Representation*>::const_iterator br(current_adp_->repesentations_.begin()), er(current_adp_->repesentations_.end()); br != er; ++br)
  {
    unsigned int score;
    if ((*br)->bandwidth_ <= bandwidth_
      && ((score = abs(static_cast<int>((*br)->width_ * (*br)->height_) - static_cast<int>(width_ * height_))
        + static_cast<unsigned int>(sqrt(bandwidth_ - (*br)->bandwidth_))) < bestScore))
    {
      bestScore = score;
      new_rep = (*br);
    }
    else if (!min_rep || (*br)->bandwidth_ < min_rep->bandwidth_)
      min_rep = (*br);
  }

  if (!new_rep)
    new_rep = min_rep;

  if (justInit)
  {
    current_rep_ = new_rep;
    return true;
  }

  if (!force && new_rep == current_rep_)
    return false;

  uint32_t segid(current_rep_ ? current_rep_->get_segment_pos(current_seg_) : 0);

  current_rep_ = new_rep;

  if (observer_)
    observer_->OnStreamChange(this, segid);

  if (tree_.isLive_) //Select an segment 8 secs befor end
  {
    size_t relativePTS = tree_.maxLiveTime_ - tree_.minLiveTime_ - 8* current_adp_->timescale_;

    size_t curSeg(current_rep_->segments_.data.size());
    while (curSeg && (current_seg_ = current_rep_->segments_[--curSeg])->range_end_ > relativePTS);
    current_seg_ = curSeg ? current_rep_->segments_[--curSeg] : 0;
  } else //Start from beginning
    current_seg_ = 0;

  return true;
}

void DASHStream::info(std::ostream &s)
{
  static const char* ts[4] = { "NoType", "Video", "Audio", "Text" };
  s << ts[type_] << " representation: " << current_rep_->url_.substr(current_rep_->url_.find_last_of('/') + 1) << " bandwidth: " << current_rep_->bandwidth_ << std::endl;
}

void DASHStream::clear()
{
  current_adp_ = 0;
  current_rep_ = 0;
}

DASHStream::~DASHStream()
{
  clear();
}
