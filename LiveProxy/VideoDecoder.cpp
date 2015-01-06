#include "stdafx.h"
#include "VideoDecoder.h"
#include "MediaQueue.h"
#include "trace.h"
#pragma unmanaged

/** Create a FrameInfo object.
*  inline helper to create FrameInfo object
\param frame_size Default frame size
*/
inline FrameInfo* FrameNew(int frame_size = 4096)
{
	FrameInfo* frame = (FrameInfo*)malloc(sizeof(FrameInfo)+frame_size);
	if (frame == NULL)
		return(NULL);
	frame->pdata = (char *)frame + sizeof(FrameInfo);//new char[frame_size];	
	frame->frameHead.FrameLen = frame_size;
	return(frame);
}

/** VideoDecoder Contrstructor.
* Allocates a AVFrame that will be reused by the decoder.  Always
* initializes the ffmpeg decoding library, to accept a H264 frame
*/
CVideoDecoder::CVideoDecoder(char const* format)
{
	try
	{
		TRACE_INFO("Register codecs");
		m_frame = avcodec_alloc_frame();
		avcodec_register_all();

		TRACE_INFO("Find codec");
		if (!strcmp(format,"H264"))
		{
			m_codec = avcodec_find_decoder(CODEC_ID_H264);
		}else{
			if (!strcmp(format,"MP4V"))
			{
				m_codec = avcodec_find_decoder(CODEC_ID_MPEG4);
			}
		}
		if ( m_codec != NULL)
		{
			TRACE_INFO("Decoder found");
		}else{
			TRACE_ERROR("Codec decoder not found");
		}

		TRACE_INFO("Allocate code context");
		m_codecContext = avcodec_alloc_context3(m_codec);
		m_codecContext->flags = 0;
       
		TRACE_INFO("open codec");
		int ret=avcodec_open2(m_codecContext, m_codec, NULL);
		if (ret < 0) 
		{
			TRACE_ERROR("Error opening codec ret=%d",ret);
		}else{
			TRACE_INFO("AV Codec found and opened");
		}
       
		m_codecContext->flags2 |= CODEC_FLAG2_CHUNKS;

	}
	catch (...)
	{
		TRACE_WARN("Ignoring Exception");
	}
}

/** VideoDecoder descructor.
 Cleans up the ffmpeg environment and 
 frees the AVFrame
*/

CVideoDecoder::~CVideoDecoder() 
{
	TRACE_INFO("Cleaning up video sing");
    if (m_frame!=NULL)
    {
        avcodec_close(m_codecContext);
        av_free(m_frame);
    }
    m_frame = NULL;

    if (m_codecContext!=NULL)
        av_free(m_codecContext);
	m_codecContext = NULL;
}

/** Decoder.
 The supplied buffer should contain an H264 video frame, then DecodeFrame
  will pass the buffer to the avcode_decode_video2 method. Once decoded we then
  use the get_picture command to convert the frame to RGB24.  The RGB24 buffer is
  then used to create a FrameInfo object which is placed on our video queue.

  \param pBuffer Memory buffer holding an H264 frame
  \param size Size of the buffer
*/
FrameInfo* CVideoDecoder::DecodeFrame(unsigned char *pBuffer, int size) 
{
		FrameInfo	*p_block=NULL;
        uint8_t startCode4[] = {0x00, 0x00, 0x00, 0x01};
        int got_frame = 0;
        AVPacket packet;

		//Initialize optional fields of a packet with default values. 
		av_init_packet(&packet);

		//set the buffer and the size	
        packet.data = pBuffer;
        packet.size = size;

        while (packet.size > sizeof(startCode4)) 
		{
			//Decode the video frame of size avpkt->size from avpkt->data into picture. 
			int len = avcodec_decode_video2(m_codecContext, m_frame, &got_frame, &packet);
			if(len<0)
			{
				TRACE_ERROR("Failed to decode video len=%d",len);
				break;
			}

			//sometime we dont get the whole frame, so move
			//forward and try again
            if ( !got_frame )
			{
				packet.size -= len;
				packet.data += len;
				continue;
			}

			//allocate a working frame to store our rgb image
            AVFrame * rgb = avcodec_alloc_frame();
			if(rgb==NULL)
			{
				TRACE_ERROR("Failed to allocate new av frame");
				return NULL;
			}

			//Allocate and return an SwsContext. 
			struct SwsContext * scale_ctx = sws_getContext(m_codecContext->width,	
				m_codecContext->height,
				m_codecContext->pix_fmt,
				m_codecContext->width,
				m_codecContext->height,
				PIX_FMT_BGR24,
				SWS_BICUBIC,
				NULL,
				NULL,
				NULL);
            if (scale_ctx == NULL)
			{
				TRACE_ERROR("Failed to get context");
				continue;
			}

			//Calculate the size in bytes that a picture of the given width and height would occupy if stored in the given picture format. 
			int numBytes = avpicture_get_size(PIX_FMT_RGB24,
				m_codecContext->width,
				m_codecContext->height);
						
			try{
				//create one of our FrameInfo objects
				p_block = FrameNew(numBytes);
				if(p_block==NULL){	

					//cleanup the working buffer
					av_free(rgb);
					sws_freeContext(scale_ctx);
					scale_ctx=NULL;
					return NULL;
				}

				//Fill our frame buffer with the rgb image
				avpicture_fill((AVPicture*)rgb, 
					(uint8_t*)p_block->pdata,
					PIX_FMT_RGB24,
					m_codecContext->width,
					m_codecContext->height);
					
				//Scale the image slice in srcSlice and put the resulting scaled slice in the image in dst. 
				sws_scale(scale_ctx, 
					m_frame->data, 
					m_frame->linesize, 
					0, 
					m_codecContext->height, 
					rgb->data, 
					rgb->linesize);
									
				//set the frame header to indicate rgb24
				p_block->frameHead.FrameType = (long)(PIX_FMT_RGB24);
				p_block->frameHead.TimeStamp = 0;
			}
			catch(...)
			{
				TRACE_ERROR("EXCEPTION: in afterGettingFrame1 ");
			}

			//cleanup the working buffer
             av_free(rgb);
			sws_freeContext(scale_ctx);

			//we got our frame no its time to move on
			break;
        }
		return p_block;
}
