#include "meta.h"
#include "../util.h"

/* RWS (Silent Hill Origins, Ghost Rider, Max Payne 2) */
VGMSTREAM * init_vgmstream_rws(STREAMFILE *streamFile) {
    VGMSTREAM * vgmstream = NULL;
    char filename[260];
    off_t start_offset;

    int loop_flag = 0;
	int channel_count;

    /* check extension, case insensitive */
    streamFile->get_name(streamFile,filename,sizeof(filename));
    if (strcasecmp("rws",filename_extension(filename))) goto fail;

    /* check header */
    if (read_32bitBE(0x00,streamFile) != 0x0D080000)
		goto fail;
#if 0
	/* check if is used as container file */
	if (read_32bitBE(0x38,streamFile) != 0x01000000)
		goto fail;
#endif

    loop_flag = 1; 
    channel_count = 2;
    
	/* build the VGMSTREAM */
    vgmstream = allocate_vgmstream(channel_count,loop_flag);
    if (!vgmstream) goto fail;

	/* fill in the vital statistics */
    start_offset = read_32bitLE(0x50,streamFile);
	vgmstream->channels = channel_count;


	switch (read_32bitLE(0x38,streamFile)) {
			case 0x01:
				vgmstream->sample_rate = read_32bitLE(0xE4,streamFile);
			    vgmstream->num_samples = read_32bitLE(0x98,streamFile)/16*28/vgmstream->channels;
				if (loop_flag) {
					vgmstream->loop_start_sample = 0;
					vgmstream->loop_end_sample = read_32bitLE(0x98,streamFile)/16*28/vgmstream->channels;
				}
			break;
			case 0x02:
				if (start_offset < 0x800) // Max Payne 2
				{
					vgmstream->sample_rate = read_32bitLE(0x178,streamFile);
				    vgmstream->num_samples = read_32bitLE(0x150,streamFile)/16*28/vgmstream->channels;
					if (loop_flag) 
					{
						vgmstream->loop_start_sample = 0;
						vgmstream->loop_end_sample = read_32bitLE(0x150,streamFile)/16*28/vgmstream->channels;
					}
				}
				else // Nana (2005)(Konami)
				{
					vgmstream->sample_rate = read_32bitLE(0x128,streamFile);
				    vgmstream->num_samples = read_32bitLE(0x7F8,streamFile)/16*28/vgmstream->channels;
					if (loop_flag) 
					{
						vgmstream->loop_start_sample = 0;
						vgmstream->loop_end_sample = read_32bitLE(0x7F8,streamFile)/16*28/vgmstream->channels;
					}				
				}
			break;
		default:
	goto fail;
}


vgmstream->coding_type = coding_PSX;


    vgmstream->layout_type = layout_interleave;
    vgmstream->interleave_block_size = read_32bitLE(0x4C,streamFile)/2;
    vgmstream->meta_type = meta_RWS;

    /* open the file for reading */
    {
        int i;
        STREAMFILE * file;
        file = streamFile->open(streamFile,filename,STREAMFILE_DEFAULT_BUFFER_SIZE);
        if (!file) goto fail;
        for (i=0;i<channel_count;i++) {
            vgmstream->ch[i].streamfile = file;

            vgmstream->ch[i].channel_start_offset=
                vgmstream->ch[i].offset=start_offset+
                vgmstream->interleave_block_size*i;

        }
    }

    return vgmstream;

    /* clean up anything we may have opened */
fail:
    if (vgmstream) close_vgmstream(vgmstream);
    return NULL;
}
