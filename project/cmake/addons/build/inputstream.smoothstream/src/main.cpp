/*
 *      Copyright (C) 2016-2016 peak3d
 *      http://www.peak3d.de
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "main.h"

#include <iostream>
#include <string.h>
#include <sstream>

#include "xbmc_addon_types.h"
#include "libXBMC_addon.h"
#include "helpers.h"
#include "kodi_vfs_types.h"
#include "SSD_dll.h"


#define SAFE_DELETE(p)       do { delete (p);     (p)=NULL; } while (0)

ADDON::CHelper_libXBMC_addon *xbmc = 0;
std::uint16_t kodiDisplayWidth(0), kodiDisplayHeight(0);

/*******************************************************
kodi host - interface for decrypter libraries
********************************************************/
class KodiHost : public SSD_HOST
{
public:
  virtual const char *GetLibraryPath() const override
  {
    return m_strLibraryPath.c_str();
  };

  virtual const char *GetProfilePath() const override
  {
    return m_strProfilePath.c_str();
  };

  virtual void* CURLCreate(const char* strURL) override
  {
    return xbmc->CURLCreate(strURL);
  };

  virtual bool CURLAddOption(void* file, CURLOPTIONS opt, const char* name, const char* value)override
  {
    const XFILE::CURLOPTIONTYPE xbmcmap[] = { XFILE::CURL_OPTION_PROTOCOL, XFILE::CURL_OPTION_HEADER };
    return xbmc->CURLAddOption(file, xbmcmap[opt], name, value);
  }

  virtual bool CURLOpen(void* file)override
  {
    return xbmc->CURLOpen(file, XFILE::READ_NO_CACHE);
  };

  virtual size_t ReadFile(void* file, void* lpBuf, size_t uiBufSize)override
  {
    return xbmc->ReadFile(file, lpBuf, uiBufSize);
  };

  virtual void CloseFile(void* file)override
  {
    return xbmc->CloseFile(file);
  };

  virtual bool CreateDirectory(const char *dir)override
  {
    return xbmc->CreateDirectory(dir);
  };

  virtual void Log(LOGLEVEL level, const char *msg)override
  {
    const ADDON::addon_log_t xbmcmap[] = { ADDON::LOG_DEBUG, ADDON::LOG_INFO, ADDON::LOG_ERROR };
    return xbmc->Log(xbmcmap[level], msg);
  };

  void SetLibraryPath(const char *libraryPath)
  {
    m_strLibraryPath = libraryPath;

    const char *pathSep(libraryPath[0] && libraryPath[1] == ':' && isalpha(libraryPath[0]) ? "\\" : "/");

    if (m_strLibraryPath.size() && m_strLibraryPath.back() != pathSep[0])
      m_strLibraryPath += pathSep;
  }

  void SetProfilePath(const char *profilePath)
  {
    m_strProfilePath = profilePath;

    const char *pathSep(profilePath[0] && profilePath[1] == ':' && isalpha(profilePath[0]) ? "\\" : "/");

    if (m_strProfilePath.size() && m_strProfilePath.back() != pathSep[0])
      m_strProfilePath += pathSep;

    //let us make cdm userdata out of the addonpath and share them between addons
    m_strProfilePath.resize(m_strProfilePath.find_last_of(pathSep[0], m_strProfilePath.length() - 2));
    m_strProfilePath.resize(m_strProfilePath.find_last_of(pathSep[0], m_strProfilePath.length() - 1));
    m_strProfilePath.resize(m_strProfilePath.find_last_of(pathSep[0], m_strProfilePath.length() - 1) + 1);

    xbmc->CreateDirectory(m_strProfilePath.c_str());
    m_strProfilePath += "cdm";
    m_strProfilePath += pathSep;
    xbmc->CreateDirectory(m_strProfilePath.c_str());
  }

private:
  std::string m_strProfilePath, m_strLibraryPath;

}kodihost;

struct addonstring
{
	addonstring(char *d) { data_ = d; };
	~addonstring() { xbmc->FreeString(data_); };
	const char* c_str() { return data_ ? data_ : ""; };
	char *data_;
};

class DummyDecrypter : public AP4_CencSingleSampleDecrypter
{
public:
  DummyDecrypter() :AP4_CencSingleSampleDecrypter(0) {};

  virtual AP4_Result DecryptSampleData(AP4_DataBuffer& data_in,
    AP4_DataBuffer& data_out,

    // always 16 bytes
    const AP4_UI08* iv,

    // pass 0 for full decryption
    unsigned int    subsample_count,

    // array of <subsample_count> integers. NULL if subsample_count is 0
    const AP4_UI16* bytes_of_cleartext_data,

    // array of <subsample_count> integers. NULL if subsample_count is 0
    const AP4_UI32* bytes_of_encrypted_data) override
  {
    return AP4_SUCCESS;
  };
};

/*******************************************************
Bento4 Streams
********************************************************/

class AP4_DASHStream : public AP4_ByteStream
{
public:
  // Constructor
  AP4_DASHStream(dash::DASHStream *dashStream) :dash_stream_(dashStream){};

  // AP4_ByteStream methods
  AP4_Result ReadPartial(void*    buffer,
    AP4_Size  bytesToRead,
    AP4_Size& bytesRead) override
  {
    bytesRead = dash_stream_->read(buffer, bytesToRead);
    return bytesRead > 0 ? AP4_SUCCESS : AP4_ERROR_READ_FAILED;
  };
  AP4_Result WritePartial(const void* buffer,
    AP4_Size    bytesToWrite,
    AP4_Size&   bytesWritten) override
  {
    /* unimplemented */
    return AP4_ERROR_NOT_SUPPORTED;
  };
  AP4_Result Seek(AP4_Position position) override
  {
    return dash_stream_->seek(position) ? AP4_SUCCESS : AP4_ERROR_NOT_SUPPORTED;
  };
  AP4_Result Tell(AP4_Position& position) override
  {
    position = dash_stream_->tell();
    return AP4_SUCCESS;
  };
  AP4_Result GetSize(AP4_LargeSize& size) override
  {
    /* unimplemented */
    return AP4_ERROR_NOT_SUPPORTED;
  };
  // AP4_Referenceable methods
  void AddReference() override {};
  void Release()override      {};
protected:
  // members
  dash::DASHStream *dash_stream_;
};

/*******************************************************
Kodi Streams implementation
********************************************************/

bool KodiDASHTree::download(const char* url)
{
  // open the file
  void* file = xbmc->CURLCreate(url);
  if (!file)
    return false;
  xbmc->CURLAddOption(file, XFILE::CURL_OPTION_PROTOCOL, "seekable", "0");
  xbmc->CURLAddOption(file, XFILE::CURL_OPTION_PROTOCOL, "acceptencoding", "gzip");
  xbmc->CURLOpen(file, XFILE::READ_CHUNKED | XFILE::READ_NO_CACHE);

  // read the file
  static const unsigned int CHUNKSIZE = 16384;
  char buf[CHUNKSIZE];
  size_t nbRead;
  while ((nbRead = xbmc->ReadFile(file, buf, CHUNKSIZE)) > 0 && ~nbRead && write_data(buf, nbRead));

  xbmc->CloseFile(file);

  xbmc->Log(ADDON::LOG_DEBUG, "Download %s finished", url);

  return nbRead == 0;
}

bool KodiDASHStream::download(const char* url)
{
  // open the file
  void* file = xbmc->CURLCreate(url);
  if (!file)
    return false;
  xbmc->CURLAddOption(file, XFILE::CURL_OPTION_PROTOCOL, "seekable" , "0");
  xbmc->CURLOpen(file, XFILE::READ_CHUNKED | XFILE::READ_NO_CACHE | XFILE::READ_AUDIO_VIDEO);

  // read the file
  char *buf = (char*)malloc(1024*1024);
  size_t nbRead, nbReadOverall = 0;;
  while ((nbRead = xbmc->ReadFile(file, buf, 1024 * 1024)) > 0 && ~nbRead && write_data(buf, nbRead)) nbReadOverall += nbRead;
  free(buf);

  double current_download_speed_ = xbmc->GetFileDownloadSpeed(file);
  //Calculate the new downloadspeed to 1MB
  static const size_t ref_packet = 1024 * 1024;
  if (nbReadOverall >= ref_packet)
    set_download_speed(current_download_speed_);
  else
  {
    double ratio = (double)nbReadOverall / ref_packet;
    set_download_speed((get_download_speed() * (1.0 - ratio)) + current_download_speed_*ratio);
  }

  xbmc->CloseFile(file);

  xbmc->Log(ADDON::LOG_DEBUG, "Download %s finished, average download speed: %0.4lf", url, get_download_speed());

  return nbRead == 0;
}

/*******************************************************
|   FragmentedSampleReader
********************************************************/
class FragmentedSampleReader : public AP4_LinearReader
{
public:

  FragmentedSampleReader(AP4_ByteStream *input, AP4_Movie *movie, AP4_Track *track,
    AP4_UI32 streamId, AP4_CencSingleSampleDecrypter *ssd)
    : AP4_LinearReader(*movie, input)
    , m_Track(track)
    , m_dts(0.0)
    , m_pts(0.0)
    , m_eos(false)
    , m_started(false)
    , m_StreamId(streamId)
    , m_SingleSampleDecryptor(ssd)
    , m_Decrypter(0)
    , m_Protected_desc(0)
    , m_Observer(0)
  {
    EnableTrack(m_Track->GetId());
    AP4_SampleDescription *desc(m_Track->GetSampleDescription(0));
    if (desc->GetType() == AP4_SampleDescription::TYPE_PROTECTED)
      m_Protected_desc = static_cast<AP4_ProtectedSampleDescription*>(desc);
  }

  ~FragmentedSampleReader()
  {
    delete m_Decrypter;
  }

  AP4_Result Start()
  {
    if (m_started)
      return AP4_SUCCESS;
    m_started = true;
    return ReadSample();
  }

  AP4_Result ReadSample()
  {
    AP4_Result result;
    if (AP4_FAILED(result = ReadNextSample(m_Track->GetId(), m_sample_, m_Protected_desc ? m_encrypted : m_sample_data_)))
    {
      if (result == AP4_ERROR_EOS) {
        m_eos = true;
        return AP4_ERROR_EOS;
      }
      else {
        return result;
      }
    }

    if (m_Protected_desc)
    {
      // Make sure that the decrypter is NOT allocating memory!
      // If decrypter and addon are compiled with different DEBUG / RELEASE
      // options freeing HEAP memory will fail.
      m_sample_data_.Reserve(m_encrypted.GetDataSize());
      if (AP4_FAILED(result = m_Decrypter->DecryptSampleData(m_encrypted, m_sample_data_, NULL)))
      {
        xbmc->Log(ADDON::LOG_ERROR, "Decrypt Sample returns failure!");
        return result;
      }
    }

    m_dts = (double)m_sample_.GetDts() / (double)m_Track->GetMediaTimeScale();
    m_pts = (double)m_sample_.GetCts() / (double)m_Track->GetMediaTimeScale();

    return AP4_SUCCESS;
  };

  void Reset(bool bEOS)
  {
    AP4_LinearReader::Reset();
    m_eos = bEOS;
  }

  bool EOS()const{ return m_eos; };
  double DTS()const{ return m_dts; };
  double PTS()const{ return m_pts; };
  const AP4_Sample &Sample()const { return m_sample_; };
  AP4_UI32 GetStreamId()const{ return m_StreamId; };
  AP4_Size GetSampleDataSize()const{ return m_sample_data_.GetDataSize(); };
  const AP4_Byte *GetSampleData()const{ return m_sample_data_.GetData(); };
  double GetDuration()const{ return (double)m_sample_.GetDuration() / (double)m_Track->GetMediaTimeScale(); };
  void SetObserver(FragmentObserver *observer) { m_Observer = observer; };
  void SetPTSOffset(uint64_t offset) { FindTracker(m_Track->GetId())->m_NextDts = offset; };
  uint64_t GetFragmentDuration() { return dynamic_cast<AP4_FragmentSampleTable*>(FindTracker(m_Track->GetId())->m_SampleTable)->GetDuration(); };

  bool TimeSeek(double pts, bool preceeding)
  {
    AP4_Ordinal sampleIndex;
    if (AP4_SUCCEEDED(SeekSample(m_Track->GetId(), static_cast<AP4_UI64>(pts*(double)m_Track->GetMediaTimeScale()), sampleIndex, preceeding)))
    {
      if (m_Decrypter)
        m_Decrypter->SetSampleIndex(sampleIndex);
      m_started = true;
      return AP4_SUCCEEDED(ReadSample());
    }
    return false;
  };

protected:
  virtual AP4_Result ProcessMoof(AP4_ContainerAtom* moof,
    AP4_Position       moof_offset,
    AP4_Position       mdat_payload_offset)
  {
    AP4_Result result;

    if (!~m_Track->GetId())
    {
      AP4_TfhdAtom* tfhd = AP4_DYNAMIC_CAST(AP4_TfhdAtom, moof->FindChild("traf/tfhd"));
      m_Track->SetId(tfhd->GetTrackId());
    }

    if (m_Observer)
      m_Observer->BeginFragment(m_StreamId);

    if (AP4_SUCCEEDED((result = AP4_LinearReader::ProcessMoof(moof, moof_offset, mdat_payload_offset))) &&  m_Protected_desc)
    {
      //Setup the decryption
      AP4_CencSampleInfoTable *sample_table;
      AP4_UI32 algorithm_id = 0;

      delete m_Decrypter;
      m_Decrypter = 0;

      AP4_ContainerAtom *traf = AP4_DYNAMIC_CAST(AP4_ContainerAtom, moof->GetChild(AP4_ATOM_TYPE_TRAF, 0));

      if (!m_Protected_desc || !traf)
        return AP4_ERROR_INVALID_FORMAT;

      if (AP4_FAILED(result = AP4_CencSampleInfoTable::Create(m_Protected_desc, traf, algorithm_id, *m_FragmentStream, moof_offset, sample_table)))
        return result;

      if (AP4_FAILED(result = AP4_CencSampleDecrypter::Create(sample_table, algorithm_id, 0, 0, 0, m_SingleSampleDecryptor, m_Decrypter)))
        return result;
    }
    if (m_Observer)
      m_Observer->EndFragment(m_StreamId);
    return result;
  }

private:
  AP4_Track *m_Track;
  AP4_UI32 m_StreamId;
  bool m_eos, m_started;
  double m_dts, m_pts;

  AP4_Sample     m_sample_;
  AP4_DataBuffer m_encrypted, m_sample_data_;

  AP4_ProtectedSampleDescription *m_Protected_desc;
  AP4_CencSingleSampleDecrypter *m_SingleSampleDecryptor;
  AP4_CencSampleDecrypter *m_Decrypter;
  FragmentObserver *m_Observer;
};

/*******************************************************
Main class Session
********************************************************/
Session *session = 0;

void Session::STREAM::disable()
{
  if (enabled)
  {
    stream_.stop();
    SAFE_DELETE(reader_);
    SAFE_DELETE(input_file_);
    SAFE_DELETE(input_);
    enabled = false;
  }
}

Session::Session(const char *strURL, const char *strLicType, const char* strLicKey, const char* strLicData, const char* profile_path)
  :single_sample_decryptor_(0)
  , mpdFileURL_(strURL)
  , license_type_(strLicType)
  , license_key_(strLicKey)
  , license_data_(strLicData)
  , profile_path_(profile_path)
  , width_(kodiDisplayWidth)
  , height_(kodiDisplayHeight)
  , last_pts_(0)
  , decrypterModule_(0)
  , decrypter_(0)
  , changed_(false)
{
  std::string fn(profile_path_ + "bandwidth.bin");
  FILE* f = fopen(fn.c_str(), "rb");
  if (f)
  {
    double val;
    fread(&val, sizeof(double), 1, f);
    dashtree_.bandwidth_ = static_cast<uint64_t>(val * 8);
    fclose(f);
  }
  else
    dashtree_.bandwidth_ = 4000000;

  int buf;
  xbmc->GetSetting("MAXRESOLUTION", (char*)&buf);
  switch (buf)
  {
  case 0:
    maxwidth_ = 0xFFFF;
    maxheight_ = 0xFFFF;
    break;
  case 2:
    maxwidth_ = 1920;
    maxheight_ = 1080;
    break;
  default:
    maxwidth_ = 1280;
    maxheight_ = 720;
  }
  if (width_ > maxwidth_)
    width_ = maxwidth_;

  if (height_ > maxheight_)
    height_ = maxheight_;
  //xbmc->GetSetting("STREAMSELECTION", (char*)&buf);
  //manual_streams_ = buf != 0;
}

Session::~Session()
{
  for (std::vector<STREAM*>::iterator b(streams_.begin()), e(streams_.end()); b != e; ++b)
    SAFE_DELETE(*b);
  streams_.clear();

  if (decrypterModule_)
  {
    dlclose(decrypterModule_);
    decrypterModule_ = 0;
    decrypter_ = 0;
  }

  std::string fn(profile_path_ + "bandwidth.bin");
  FILE* f = fopen(fn.c_str(), "wb");
  if (f)
  {
    double val(dashtree_.get_average_download_speed());
    fwrite((const char*)&val, sizeof(double), 1, f);
    fclose(f);
  }
}

void Session::GetSupportedDecrypterURN(std::pair<std::string, std::string> &urn)
{
  typedef SSD_DECRYPTER *(*CreateDecryptorInstanceFunc)(SSD_HOST *host, uint32_t version);

  char specialpath[1024];
  if (!xbmc->GetSetting("DECRYPTERPATH", specialpath))
  {
    xbmc->Log(ADDON::LOG_DEBUG, "DECRYPTERPATH not specified in settings.xml");
    return;
  }
  addonstring path(xbmc->TranslateSpecialProtocol(specialpath));
  kodihost.SetLibraryPath(path.c_str());

  VFSDirEntry *items(0);
  unsigned int num_items(0);

  xbmc->Log(ADDON::LOG_DEBUG, "Searching for decrypters in: %s", path.c_str());

  if (!xbmc->GetDirectory(path.c_str(), "", &items, &num_items))
    return;

  for (unsigned int i(0); i < num_items; ++i)
  {
    if (strncmp(items[i].label, "ssd_", 4) && strncmp(items[i].label, "libssd_", 7))
      continue;

    void * mod(dlopen(items[i].path, RTLD_LAZY));
    if (mod)
    {
      CreateDecryptorInstanceFunc startup;
      if ((startup = (CreateDecryptorInstanceFunc)dlsym(mod, "CreateDecryptorInstance")))
      {
        SSD_DECRYPTER *decrypter = startup(&kodihost, SSD_HOST::version);
        const char *suppUrn(0);

        if (decrypter && (suppUrn = decrypter->Supported(license_type_.c_str(), license_key_.c_str())))
        {
          xbmc->Log(ADDON::LOG_DEBUG, "Found decrypter: %s", items[i].path);
          decrypterModule_ = mod;
          decrypter_ = decrypter;
          urn.first = suppUrn;
          break;
        }
      }
      dlclose(mod);
    }
  }
  xbmc->FreeDirectory(items, num_items);

}

AP4_CencSingleSampleDecrypter *Session::CreateSingleSampleDecrypter(AP4_DataBuffer &streamCodec)
{
  if (decrypter_)
    return decrypter_->CreateSingleSampleDecrypter(streamCodec);
  return 0;
};

/*----------------------------------------------------------------------
|   initialize
+---------------------------------------------------------------------*/

bool Session::initialize()
{
  // Get URN's wich are supported by this addon
  if (!license_type_.empty())
  {
    GetSupportedDecrypterURN(dashtree_.adp_pssh_);
    xbmc->Log(ADDON::LOG_DEBUG, "Supported URN: %s", dashtree_.adp_pssh_.first.c_str());
  }

  // Open mpd file
  size_t paramPos = mpdFileURL_.find('?');
  dashtree_.base_url_ = (paramPos == std::string::npos)? mpdFileURL_:mpdFileURL_.substr(0, paramPos);

  paramPos = dashtree_.base_url_.find_last_of('/', dashtree_.base_url_.length());
  if (paramPos == std::string::npos)
  {
    xbmc->Log(ADDON::LOG_ERROR, "Invalid mpdURL: / expected (%s)", mpdFileURL_.c_str());
    return false;
  }
  dashtree_.base_url_.resize(paramPos + 1);

  if (!dashtree_.open(mpdFileURL_.c_str()) || dashtree_.empty())
  {
    xbmc->Log(ADDON::LOG_ERROR, "Could not open / parse mpdURL (%s)", mpdFileURL_.c_str());
    return false;
  }
  xbmc->Log(ADDON::LOG_INFO, "Successfully parsed .mpd file. Download speed: %0.4f Bytes/s", dashtree_.download_speed_);

  if (dashtree_.encryptionState_ == dash::DASHTree::ENCRYTIONSTATE_ENCRYPTED)
  {
    xbmc->Log(ADDON::LOG_ERROR, "Unable to handle decryption. Unsupported!");
    return false;
  }

  uint32_t min_bandwidth(0), max_bandwidth(0);
  {
    int buf;
    xbmc->GetSetting("MINBANDWIDTH", (char*)&buf); min_bandwidth = buf;
    xbmc->GetSetting("MAXBANDWIDTH", (char*)&buf); max_bandwidth = buf;
  }

  // create SESSION::STREAM objects. One for each AdaptationSet
  unsigned int i(0);
  const dash::DASHTree::AdaptationSet *adp;

  for (std::vector<STREAM*>::iterator b(streams_.begin()), e(streams_.end()); b != e; ++b)
    SAFE_DELETE(*b);
  streams_.clear();

  while ((adp = dashtree_.GetAdaptationSet(i++)))
  {
    streams_.push_back(new STREAM(dashtree_, adp->type_));
    STREAM &stream(*streams_.back());
    stream.stream_.prepare_stream(adp, width_, height_, min_bandwidth, max_bandwidth);

    switch (adp->type_)
    {
    case dash::DASHTree::VIDEO:
      stream.info_.m_streamType = INPUTSTREAM_INFO::TYPE_VIDEO;
      break;
    case dash::DASHTree::AUDIO:
      stream.info_.m_streamType = INPUTSTREAM_INFO::TYPE_AUDIO;
      break;
    case dash::DASHTree::TEXT:
      stream.info_.m_streamType = INPUTSTREAM_INFO::TYPE_TELETEXT;
      break;
    default:
      break;
    }
    stream.info_.m_pID = i;
    strcpy(stream.info_.m_language, adp->language_.c_str());

    UpdateStream(stream);

  }

  // Try to initialize an SingleSampleDecryptor
#if 1
  if (dashtree_.encryptionState_)
  {
    if (dashtree_.protection_key_.size()!=16 || license_data_.empty())
      return false;

    uint8_t ld[1024];
    unsigned int ld_size(1024);
    b64_decode(license_data_.c_str(), license_data_.size(), ld, ld_size);

    const uint8_t *uuid((uint8_t*)strstr((const char*)ld, "{UUID}"));
    unsigned int license_size = uuid ? ld_size + 36 -6: ld_size;

    //Build up proto header
    AP4_DataBuffer init_data;
    init_data.Reserve(512);
    uint8_t *protoptr(init_data.UseData());
    *protoptr++ = 18; //id=16>>3=2, type=2(flexlen)
    *protoptr++ = 16; //length of key
    memcpy(protoptr, dashtree_.protection_key_.data(), 16);
    protoptr += 16;
    //-----------
    *protoptr++ = 34;//id=32>>3=4, type=2(flexlen)
    do {
      *protoptr++ = static_cast<uint8_t>(license_size & 127);
      license_size >>= 7;
      if (license_size)
        *(protoptr - 1) |= 128;
      else
        break;
    } while (1);
    if (uuid)
    {
      static const uint8_t hexmap[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
      memcpy(protoptr, ld, uuid - ld);
      protoptr += uuid - ld;
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[3]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[3]) & 15];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[2]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[2]) & 15];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[1]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[1]) & 15];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[0]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[0]) & 15];
      *protoptr++ = '-';
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[5]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[5]) & 15];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[4]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[4]) & 15];
      *protoptr++ = '-';
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[7]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[7]) & 15];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[6]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[6]) & 15];
      *protoptr++ = '-';
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[8]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[8]) & 15];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[9]) >> 4];
      *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[9]) & 15];
      *protoptr++ = '-';
      for (i = 10; i < 16; ++i)
      {
        *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[i]) >> 4];
        *protoptr++ = hexmap[(uint8_t)(dashtree_.protection_key_.data()[i]) & 15];
      }
      unsigned int sizeleft = ld_size - ((uuid - ld) + 6);
      memcpy(protoptr, uuid+6, sizeleft);
      protoptr += sizeleft;
    }
    else
    {
      memcpy(protoptr, ld, ld_size);
      protoptr += ld_size;
    }
    init_data.SetDataSize(protoptr - init_data.UseData());
    return (single_sample_decryptor_ = CreateSingleSampleDecrypter(init_data))!=0;
  }
#else
  return (single_sample_decryptor_ = new DummyDecrypter()) != 0;

  //dashtree_.encryptionState_ = 0;
#endif
  return true;
}

void Session::UpdateStream(STREAM &stream)
{
  const dash::DASHTree::Representation *rep(stream.stream_.getRepresentation());

  stream.info_.m_Width = rep->width_;
  stream.info_.m_Height = rep->height_;
  stream.info_.m_Aspect = rep->aspect_;

  // we currently use only the first track!
  std::string::size_type pos = rep->codecs_.find(",");
  if (pos == std::string::npos)
    pos = rep->codecs_.size();

  strncpy(stream.info_.m_codecInternalName, rep->codecs_.c_str(), pos);
  stream.info_.m_codecInternalName[pos] = 0;

  if (rep->codecs_.find("AAC") == 0)
    strcpy(stream.info_.m_codecName, "aac");
  else if (rep->codecs_.find("ec-3") == 0 || rep->codecs_.find("ac-3") == 0)
    strcpy(stream.info_.m_codecName, "eac3");
  else if (rep->codecs_.find("AVC") == 0
    || rep->codecs_.find("H264") == 0)
    strcpy(stream.info_.m_codecName, "h264");
  else if (rep->codecs_.find("hevc") == 0)
    strcpy(stream.info_.m_codecName, "hevc");

  stream.info_.m_FpsRate = rep->fpsRate_;
  stream.info_.m_FpsScale = rep->fpsScale_;
  stream.info_.m_SampleRate = rep->samplingRate_;
  stream.info_.m_Channels = rep->channelCount_;
  stream.info_.m_Bandwidth = rep->bandwidth_;

  stream.info_.m_ExtraData = reinterpret_cast<const uint8_t*>(rep->codec_extra_data_.data());
  stream.info_.m_ExtraSize = rep->codec_extra_data_.size();
}


FragmentedSampleReader *Session::GetNextSample()
{
  STREAM *res(0);
  for (std::vector<STREAM*>::const_iterator b(streams_.begin()), e(streams_.end()); b != e; ++b)
    if ((*b)->enabled && !(*b)->reader_->EOS()
    && (!res || (*b)->reader_->DTS() < res->reader_->DTS()))
        res = *b;

  if (res && AP4_SUCCEEDED(res->reader_->Start()))
  {
    last_pts_ = res->reader_->PTS();
    return res->reader_;
  }
  return 0;
}

bool Session::SeekTime(double seekTime, unsigned int streamId, bool preceeding)
{
  bool ret(false);

  for (std::vector<STREAM*>::const_iterator b(streams_.begin()), e(streams_.end()); b != e; ++b)
    if ((*b)->enabled && (streamId == 0 || (*b)->info_.m_pID == streamId))
    {
      bool bReset;
      if ((*b)->stream_.seek_time(seekTime, last_pts_, bReset))
      {
        if (bReset)
          (*b)->reader_->Reset(false);
        if (!(*b)->reader_->TimeSeek(seekTime, preceeding))
          (*b)->reader_->Reset(true);
        else
        {
          xbmc->Log(ADDON::LOG_INFO, "seekTime(%0.4f) for Stream:%d continues at %0.4f", seekTime, (*b)->info_.m_pID, (*b)->reader_->PTS());
          ret = true;
        }
      }
      else
        (*b)->reader_->Reset(true);
    }
  return ret;
}

void Session::BeginFragment(AP4_UI32 streamId)
{
  STREAM *s(streams_[streamId - 1]);
  s->reader_->SetPTSOffset(s->stream_.GetPTSOffset());
}

void Session::EndFragment(AP4_UI32 streamId)
{
  STREAM *s(streams_[streamId - 1]);
  dashtree_.SetFragmentDuration(s->stream_.getAdaptation(), s->stream_.getSegmentPos(), s->reader_->GetFragmentDuration());
}


/***************************  Interface *********************************/

#include "kodi_inputstream_dll.h"
#include "libKODI_inputstream.h"

extern "C" {

  ADDON_STATUS curAddonStatus = ADDON_STATUS_UNKNOWN;
  CHelper_libKODI_inputstream *ipsh = 0;

  /***********************************************************
  * Standard AddOn related public library functions
  ***********************************************************/

  ADDON_STATUS ADDON_Create(void* hdl, void* props)
  {
    // initialize globals
    session = nullptr;
    kodiDisplayWidth = 1280;
    kodiDisplayHeight = 720;

    if (!hdl)
      return ADDON_STATUS_UNKNOWN;

    xbmc = new ADDON::CHelper_libXBMC_addon;
    if (!xbmc->RegisterMe(hdl))
    {
      SAFE_DELETE(xbmc);
      return ADDON_STATUS_PERMANENT_FAILURE;
    }

    ipsh = new CHelper_libKODI_inputstream;
    if (!ipsh->RegisterMe(hdl))
    {
      SAFE_DELETE(xbmc);
      SAFE_DELETE(ipsh);
      return ADDON_STATUS_PERMANENT_FAILURE;
    }

    xbmc->Log(ADDON::LOG_DEBUG, "ADDON_Create()");

    curAddonStatus = ADDON_STATUS_UNKNOWN;

    //if (XBMC->GetSetting("host", buffer))

    curAddonStatus = ADDON_STATUS_OK;
    return curAddonStatus;
  }

  ADDON_STATUS ADDON_GetStatus()
  {
    return curAddonStatus;
  }

  void ADDON_Destroy()
  {
    xbmc->Log(ADDON::LOG_DEBUG, "ADDON_Destroy()");
    SAFE_DELETE(session);
    SAFE_DELETE(xbmc);
    SAFE_DELETE(ipsh);
  }

  bool ADDON_HasSettings()
  {
    xbmc->Log(ADDON::LOG_DEBUG, "ADDON_HasSettings()");
    return false;
  }

  unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
  {
    xbmc->Log(ADDON::LOG_DEBUG, "ADDON_GetSettings()");
    return 0;
  }

  ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
  {
    xbmc->Log(ADDON::LOG_DEBUG, "ADDON_SetSettings()");
    return ADDON_STATUS_OK;
  }

  void ADDON_Stop()
  {
  }

  void ADDON_FreeSettings()
  {
  }

  void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
  {
  }

  /***********************************************************
  * InputSteam Client AddOn specific public library functions
  ***********************************************************/

  bool Open(INPUTSTREAM& props)
  {
    xbmc->Log(ADDON::LOG_DEBUG, "Open()");

    const char *lt(""), *lk(""), *ld("");
    for (unsigned int i(0); i < props.m_nCountInfoValues; ++i)
    {
      if (strcmp(props.m_ListItemProperties[i].m_strKey, "inputstream.smoothstream.license_type") == 0)
      {
        xbmc->Log(ADDON::LOG_DEBUG, "found inputstream.smoothstream.license_type: %s", props.m_ListItemProperties[i].m_strValue);
        lt = props.m_ListItemProperties[i].m_strValue;
      }
      else if (strcmp(props.m_ListItemProperties[i].m_strKey, "inputstream.smoothstream.license_key") == 0)
      {
        xbmc->Log(ADDON::LOG_DEBUG, "found inputstream.smoothstream.license_key: [not shown]");
        lk = props.m_ListItemProperties[i].m_strValue;
      }
      else if (strcmp(props.m_ListItemProperties[i].m_strKey, "inputstream.smoothstream.license_data") == 0)
      {
        xbmc->Log(ADDON::LOG_DEBUG, "found inputstream.smoothstream.license_data: [not shown]");
        ld = props.m_ListItemProperties[i].m_strValue;
      }
    }

    kodihost.SetProfilePath(props.m_profileFolder);

    session = new Session(props.m_strURL, lt, lk, ld, props.m_profileFolder);

    if (!session->initialize())
    {
      SAFE_DELETE(session);
      return false;
    }
    return true;
  }

  void Close(void)
  {
    xbmc->Log(ADDON::LOG_DEBUG, "Close()");
    SAFE_DELETE(session);
  }

  const char* GetPathList(void)
  {
    return "";
  }

  struct INPUTSTREAM_IDS GetStreamIds()
  {
    xbmc->Log(ADDON::LOG_DEBUG, "GetStreamIds()");
    INPUTSTREAM_IDS iids;

    if(session)
    {
        iids.m_streamCount = session->GetStreamCount();
        for (unsigned int i(0); i < iids.m_streamCount;++i)
          iids.m_streamIds[i] = i+1;
    } else
        iids.m_streamCount = 0;
    return iids;
  }

  struct INPUTSTREAM_CAPABILITIES GetCapabilities()
  {
    xbmc->Log(ADDON::LOG_DEBUG, "GetCapabilities()");
    INPUTSTREAM_CAPABILITIES caps;
    caps.m_supportsIDemux = true;
    caps.m_supportsIPosTime = false;
    caps.m_supportsIDisplayTime = true;
    caps.m_supportsSeek = true;
    caps.m_supportsPause = true;
    return caps;
  }

  struct INPUTSTREAM_INFO GetStream(int streamid)
  {
    static struct INPUTSTREAM_INFO dummy_info = {
      INPUTSTREAM_INFO::TYPE_NONE, "", "", 0, 0, 0, 0, "",
      0, 0, 0, 0, 0.0f,
      0, 0, 0, 0, 0 };

    xbmc->Log(ADDON::LOG_DEBUG, "GetStream(%d)", streamid);

    Session::STREAM *stream(session->GetStream(streamid));
    if (stream)
      return stream->info_;

    return dummy_info;
  }

  void EnableStream(int streamid, bool enable)
  {
    xbmc->Log(ADDON::LOG_DEBUG, "EnableStream (%d -> %s)", streamid, enable?"true":"false");

    if (!session)
      return;

    Session::STREAM *stream(session->GetStream(streamid));

    if (!stream)
      return;

    if (enable)
    {
      if (stream->enabled)
        return;

      stream->enabled = true;

      stream->stream_.start_stream(0);
      const dash::DASHTree::Representation *rep(stream->stream_.getRepresentation());
      stream->stream_.select_stream(true, false/*, stream->info_.m_pID >> 16*/);
      if (rep != stream->stream_.getRepresentation())
      {
        session->UpdateStream(*stream);
        session->CheckChange(true);
      }

      stream->input_ = new AP4_DASHStream(&stream->stream_);

      AP4_Movie* movie = NULL;
      static const AP4_Track::Type TIDC[dash::DASHTree::STREAM_TYPE_COUNT] =
      { AP4_Track::TYPE_UNKNOWN, AP4_Track::TYPE_VIDEO, AP4_Track::TYPE_AUDIO, AP4_Track::TYPE_TEXT };

      if (1/*!stream->stream_.getRepresentation()->has_initialization()*/)
      {
        //We'll create a Movie out of the things we got from manifest file
        //note: movie will be deleted in destructor of stream->input_file_
        movie = new AP4_Movie();

        AP4_SyntheticSampleTable* sample_table = new AP4_SyntheticSampleTable();
        AP4_SampleDescription *sample_descryption = new AP4_SampleDescription(AP4_SampleDescription::TYPE_UNKNOWN, 0, 0);
        if (session->IsEncrypted())
        {
          static const AP4_UI08 default_key[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
          AP4_ContainerAtom schi(AP4_ATOM_TYPE_SCHI);
          schi.AddChild(new AP4_TencAtom(AP4_CENC_ALGORITHM_ID_CTR, 8, default_key));
          sample_descryption = new AP4_ProtectedSampleDescription(0, sample_descryption, 0, AP4_PROTECTION_SCHEME_TYPE_PIFF, 0, "", &schi);
        }
        sample_table->AddSampleDescription(sample_descryption);

        movie->AddTrack(new AP4_Track(TIDC[stream->stream_.get_type()], sample_table, ~0, stream->stream_.getRepresentation()->timescale_, 0, stream->stream_.getRepresentation()->timescale_, 0, "", 0, 0));
        //Create a dumy MOOV Atom to tell Bento4 its a fragmented stream
        AP4_MoovAtom *moov = new AP4_MoovAtom();
        moov->AddChild(new AP4_ContainerAtom(AP4_ATOM_TYPE_MVEX));
        movie->SetMoovAtom(moov);
      }
      stream->input_file_ = new AP4_File(*stream->input_, AP4_DefaultAtomFactory::Instance, true, movie);
      movie = stream->input_file_->GetMovie();
      if (movie == NULL)
      {
        xbmc->Log(ADDON::LOG_ERROR, "No MOOV found in stream's initialization");
        return stream->disable();
      }

      AP4_Track *track = movie->GetTrack(TIDC[stream->stream_.get_type()]);
      if (!track)
      {
        xbmc->Log(ADDON::LOG_ERROR, "No suitable track found in stream");
        return stream->disable();
      }

      stream->reader_ = new FragmentedSampleReader(
        stream->input_,
        movie, track,
        streamid,
        session->GetSingleSampleDecryptor());

      stream->reader_->SetObserver(dynamic_cast<FragmentObserver*>(session));

      return;
    }
    return stream->disable();
  }

  int ReadStream(unsigned char*, unsigned int)
  {
    return -1;
  }

  int64_t SeekStream(int64_t, int)
  {
    return -1;
  }

  int64_t PositionStream(void)
  {
    return -1;
  }

  int64_t LengthStream(void)
  {
    return -1;
  }

  void DemuxReset(void)
  {
  }

  void DemuxAbort(void)
  {
  }

  void DemuxFlush(void)
  {
  }

  DemuxPacket* __cdecl DemuxRead(void)
  {
    if (!session)
      return NULL;

    FragmentedSampleReader *sr(session->GetNextSample());

    if (session->CheckChange())
    {
      DemuxPacket *p = ipsh->AllocateDemuxPacket(0);
      p->iStreamId = DMX_SPECIALID_STREAMCHANGE;
      xbmc->Log(ADDON::LOG_DEBUG, "DMX_SPECIALID_STREAMCHANGE");
      return p;
    }

    if (sr)
    {
      const AP4_Sample &s(sr->Sample());
      DemuxPacket *p = ipsh->AllocateDemuxPacket(sr->GetSampleDataSize());
      p->dts = sr->DTS() * 1000000;
      p->pts = sr->PTS() * 1000000;
      p->duration = sr->GetDuration() * 1000000;
      p->iStreamId = sr->GetStreamId();
      p->iGroupId = 0;
      p->iSize = sr->GetSampleDataSize();
      memcpy(p->pData, sr->GetSampleData(), p->iSize);

      //xbmc->Log(ADDON::LOG_DEBUG, "DTS: %0.4f, PTS:%0.4f, ID: %u SZ: %d", p->dts, p->pts, p->iStreamId, p->iSize);

      sr->ReadSample();
      return p;
    }
    return NULL;
  }

  bool DemuxSeekTime(int time, bool backwards, double *startpts)
  {
    if (!session)
      return false;

    xbmc->Log(ADDON::LOG_INFO, "DemuxSeekTime (%d)", time);

    return session->SeekTime(static_cast<double>(time)*0.001f, 0, !backwards);
  }

  void DemuxSetSpeed(int speed)
  {

  }

  //callback - will be called from kodi
  void SetVideoResolution(int width, int height)
  {
    xbmc->Log(ADDON::LOG_INFO, "SetVideoResolution (%d x %d)", width, height);
    if (session)
      session->SetVideoResolution(width, height);
    else
    {
      kodiDisplayWidth = width;
      kodiDisplayHeight = height;
    }
  }

  int GetTotalTime()
  {
    if (!session)
      return 0;

    return static_cast<int>(session->GetTotalTime()*1000);
  }

  int GetTime()
  {
    if (!session)
      return 0;

    return static_cast<int>(session->GetPTS() * 1000);
  }

  bool CanPauseStream(void)
  {
    return !session->IsLive();
  }

  bool CanSeekStream(void)
  {
    return !session->IsLive();
  }

  bool PosTime(int)
  {
    return false;
  }

  void SetSpeed(int)
  {
  }

  void PauseStream(double)
  {
  }

  bool IsRealTimeStream(void)
  {
    return false;
  }

}//extern "C"
