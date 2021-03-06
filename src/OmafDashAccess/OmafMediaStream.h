/*
 * Copyright (c) 2019, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 */

//!
//! \file:   OmafMediaStream.h
//! \brief:
//! \detail:
//! Created on May 22, 2019, 2:22 PM
//!

#ifndef OMAFMEDIASTREAM_H
#define OMAFMEDIASTREAM_H

#include <memory>

#include "general.h"

#include "MediaPacket.h"
#include "OmafAdaptationSet.h"
#include "OmafExtractor.h"
#include "OmafReader.h"
#include "OmafTilesStitch.h"
#include <mutex>

VCD_OMAF_BEGIN

class OmafReaderManager;
class OmafDashSegmentClient;

class OmafMediaStream {
 public:
  //!
  //! \brief  construct
  //!
  OmafMediaStream();

  //!
  //! \brief  de-construct
  //!
  virtual ~OmafMediaStream();

 public:
  void SetOmafReaderMgr(std::shared_ptr<OmafReaderManager> mgr) noexcept;
  //!
  //! \brief update the start number of the segment for dynamical mode
  //! \param nAvailableStartTime used to calculate start number when accessed
  //!        mpd the first: (now - nAvailableStartTime)/segment_duration + Adaption_Strart_number
  //! \return
  int UpdateStartNumber(uint64_t nAvailableStartTime);

  int SetupSegmentSyncer(const OmafDashParams& params);
  //!
  //! \brief  download initialize segment for each AdaptationSet
  //!
  int DownloadInitSegment();

  //!
  //! \brief  download all segments for all AdaptationSets.
  //!
  int DownloadSegments();

  //!
  //! \brief  Add extractor Adaptation Set
  //!
  int AddExtractor(OmafExtractor* pAS);

  //!
  //! \brief  Add normal Adaptation Set
  //!
  int AddAdaptationSet(OmafAdaptationSet* pAS);

  //!
  //! \brief  Initialize the stream
  //!
  int InitStream(std::string type);

  //!
  //! \brief  SetMainAdaptationSet if there is
  //!
  void SetMainAdaptationSet(OmafAdaptationSet* as) { mMainAdaptationSet = as; }

  //!
  //! \brief  SetExtratorAdaptationSet if there is
  //!
  void SetExtratorAdaptationSet(OmafAdaptationSet* as) { mExtratorAdaptationSet = as; }

  //!
  //! \brief  Seek to special segment and is is valid in static mode
  //!
  int SeekTo(int seg_num);

  //!
  //! \brief  Set EOS for the stream
  //!
  int SetEOS(bool eos) {
    m_bEOS = eos;
    return 0;
  };

  //!
  //! \brief  get all extractors relative to this stream
  //!
  std::map<int, OmafExtractor*> GetExtractors() { return mExtractors; };

  //!
  //! \brief  get all Adaptation set relative to this stream
  //!
  std::map<int, OmafAdaptationSet*> GetMediaAdaptationSet() { return mMediaAdaptationSet; };

  //!
  //! \brief  Update selected extractor after viewport changed
  //!
  int UpdateEnabledExtractors(std::list<OmafExtractor*> extractors);

  //!
  //! \brief  Update selected tile tracks after viewport changed
  //!
  int UpdateEnabledTileTracks(std::map<int, OmafAdaptationSet*> selectedTiles);

  int EnableAllAudioTracks();

  //!
  //! \brief  Get count of tracks
  //!
  int GetTrackCount();

  //!
  //! \brief  Get/Set stream ID
  //!
  void SetStreamID(int streamID) { mStreamID = streamID; };
  int GetStreamID() { return mStreamID; };

  //!
  //! \brief  get Stream information
  //!
  DashStreamInfo* GetStreamInfo() { return m_pStreamInfo; };

  //!
  //! \brief  get current selected extractors
  //!
  std::list<OmafExtractor*> GetEnabledExtractor() {
    std::lock_guard<std::mutex> lock(mCurrentMutex);
    std::list<OmafExtractor*> enabledExtractor(mCurrentExtractors.begin(), mCurrentExtractors.end());
    return enabledExtractor;
  };

  //std::map<int, OmafAdaptationSet*> GetSelectedTileTracks() {
  //  std::lock_guard<std::mutex> lock(mCurrentMutex);
  //  std::map<int, OmafAdaptationSet*> selectedTileTracks = m_selectedTileTracks.front();
  //  return selectedTileTracks;
  //}

  int32_t GetExtractorSize() {
    std::lock_guard<std::mutex> lock(mCurrentMutex);
    int32_t size = mCurrentExtractors.size();
    return size;
  };

  int32_t GetTotalExtractorSize() { return mExtractors.size(); };

  void ClearEnabledExtractors() { mCurrentExtractors.clear(); };

  OmafExtractor* AddEnabledExtractor(int extractorTrackIdx) {
    auto it = mExtractors.find(extractorTrackIdx);
    if (it != mExtractors.end()) {
      mCurrentExtractors.push_back(it->second);
      return (it->second);
    } else {
      return NULL;
    }
  };

  //!
  //! \brief  Check whether extractor tracks exists
  //!
  bool HasExtractor() { return !(0 == mExtractors.size()); };

  //!
  //! \brief  Get segment duration
  //!
  uint64_t GetSegmentDuration() { return m_pStreamInfo ? m_pStreamInfo->segmentDuration : 0; };

  uint32_t GetStreamWidth() { return m_pStreamInfo ? m_pStreamInfo->width : 0; };

  uint32_t GetStreamHeight() { return m_pStreamInfo ? m_pStreamInfo->height : 0; };

  uint32_t GetStreamHighResWidth() { return m_pStreamInfo ? m_pStreamInfo->source_resolution[0].width : 0; };

  uint32_t GetStreamHighResHeight() { return m_pStreamInfo ? m_pStreamInfo->source_resolution[0].height : 0; };

  uint32_t GetRowSize() { return m_pStreamInfo ? m_pStreamInfo->tileRowNum : 0; };

  uint32_t GetColSize() { return m_pStreamInfo ? m_pStreamInfo->tileColNum : 0; };

  uint32_t GetSegmentNumber() { return m_activeSegmentNum;};

  bool IsExtractorEnabled() { return m_enabledExtractor; };

  void SetEnabledExtractor(bool enabledExtractor) { m_enabledExtractor = enabledExtractor; };

  void SetSources(std::map<uint32_t, SourceInfo> sources) { m_sources = sources; };

  void SetNeedVideoParams(bool needParams) { m_needParams = needParams; };

  void SetMaxStitchResolution(uint32_t width, uint32_t height) { m_stitch->SetMaxStitchResolution(width, height); };

  void SetSegmentNumber( uint32_t seg_num ) { m_activeSegmentNum = seg_num; } ;

  std::list<MediaPacket*> GetOutTilesMergedPackets();

  MediaType GetStreamMediaType() { return m_pStreamInfo->stream_type; };

  void Close();

 private:
  //!
  //! \brief  UpdateStreamInfo
  //!
  OMAF_STATUS UpdateStreamInfo();

  //!
  //! \brief  SetupExtratorDependency
  //!
  void SetupExtratorDependency();

  int32_t StartTilesStitching();

  static void* TilesStitchingThread(void* pThis);

  int32_t TilesStitching();

private:
    OmafMediaStream& operator=(const OmafMediaStream& other) { return *this; };
    OmafMediaStream(const OmafMediaStream& other) { /* do not create copies */ };

 private:
  //<! Adaptation Set list for tiles
  std::map<int, OmafAdaptationSet*> mMediaAdaptationSet;
  //<! Adaptation Set list for extractor
  std::map<int, OmafExtractor*> mExtractors;
  //<! the current extractors to be dealt with
  std::list<OmafExtractor*> mCurrentExtractors;
  //<! the main AdaptationSet, it can be exist or not
  OmafAdaptationSet* mMainAdaptationSet;
  //<! the Extrator AdaptationSet
  OmafAdaptationSet* mExtratorAdaptationSet;
  //<! stream ID
  int mStreamID;
  DashStreamInfo* m_pStreamInfo;
  //<! the information of the stream
  //<! for synchronization
  std::mutex mMutex;
  //<! for synchronization of mCurrentExtractors and m_selectedTileTracks
  std::mutex mCurrentMutex;
  //<! flag for end of stream
  bool m_bEOS;
  OmafDashSourceSyncHelper syncer_helper_;
  std::shared_ptr<OmafReaderManager> omaf_reader_mgr_;
  //<! flag for enabling/disabling extractor track
  bool m_enabledExtractor;
  //<! map of selected tile tracks based on viewport when disabling extractor track
  //std::list<std::map<int, OmafAdaptationSet*>> m_selectedTileTracks;
  uint64_t m_tileSelTimeLine;
  std::map<uint64_t, std::map<int, OmafAdaptationSet*>> m_selectedTileTracks;

  bool m_hasTileTracksSelected;
  //<! map of video sources for the media stream
  std::map<uint32_t, SourceInfo> m_sources;
  //<! tiles stitching thread ID
  pthread_t m_stitchThread;
  //<! mutex for output tiles merged media packet list
  std::mutex m_packetsMutex;
  //<! list of output tiles merged media packets
  std::list<std::list<MediaPacket*>> m_mergedPackets;

  bool m_needParams;
  //<! tiles stitch handle
  OmafTilesStitch* m_stitch;

  int m_status;
  uint64_t m_currFrameIdx;  //<! the frame index which is currently processed

  uint32_t m_activeSegmentNum;
};

VCD_OMAF_END;

#endif /* MEDIASTREAM_H */
