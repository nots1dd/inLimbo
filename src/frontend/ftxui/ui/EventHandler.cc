#include "frontend/ftxui/ui/EventHandler.hpp"
#include "query/SongMap.hpp"

using namespace ftxui;

namespace frontend::tui::ui
{

EventHandler::EventHandler(UIScreen& activeScreen, state::library::LibraryState& libraryState,
                           state::now_playing::NowPlayingState& nowState,
                           state::queue::QueueState&            queueState,
                           managers::ThreadManager& threadManager, TS_SongMap* songMap,
                           mpris::Service* mpris)
    : m_activeScreen(activeScreen), m_libraryState(libraryState), m_nowState(nowState),
      m_queueState(queueState), m_threadManager(threadManager), m_songMap(songMap), m_mpris(mpris)
{
}

// delta in range 0.0 to 1.5 (will be clamped)
void EventHandler::adjustVolume(float delta)
{
  float vol = m_audioPtr->getVolume();
  vol       = std::clamp(vol + delta, 0.0f, 1.5f);
  m_audioPtr->setVolume(vol);
  m_volume = vol;
}

void EventHandler::toggleMute()
{
  float vol = m_audioPtr->getVolume();

  if (vol == 0.0f)
    m_audioPtr->setVolume(m_volume);
  else
  {
    m_volume = vol;
    m_audioPtr->setVolume(0.0f);
  }
}

auto EventHandler::handle(Event e) -> bool
{
  if (e == Event::Character('q'))
  {
    ScreenInteractive::Active()->Exit();
    return true;
  }

  switch (m_activeScreen)
  {
    case UIScreen::Main:
    {
      if (e == Event::Character('\t'))
      {
        m_libraryState.toggleFocus();
        return true;
      }

      if (e == Event::ArrowDown)
      {
        m_libraryState.moveSelection(1);
        return true;
      }

      if (e == Event::ArrowUp)
      {
        m_libraryState.moveSelection(-1);
        return true;
      }

      if (e == Event::Character('g')) // top
      {
        if (m_libraryState.focusOnArtists())
        {
          m_libraryState.selected_artist = 0;
          m_libraryState.rebuildForSelectedArtist(0);
        }
        else
        {
          m_libraryState.selected_album_index = 0;
        }
        return true;
      }

      if (e == Event::Character('G')) // bottom
      {
        if (m_libraryState.focusOnArtists())
        {
          if (!m_libraryState.artists.empty())
          {
            m_libraryState.selected_artist = static_cast<int>(m_libraryState.artists.size()) - 1;
            m_libraryState.rebuildForSelectedArtist(m_libraryState.selected_artist);
          }
        }
        else
        {
          if (!m_libraryState.view_song_objects.empty())
          {
            m_libraryState.selected_album_index =
              static_cast<int>(m_libraryState.view_song_objects.size()) - 1;
          }
        }
        return true;
      }

      if (e == Event::Return)
      {
        playSelected();
        return true;
      }

      if (e == Event::Character('2'))
      {
        m_activeScreen = UIScreen::NowPlaying;
        return true;
      }

      if (e == Event::Character('3'))
      {
        m_activeScreen = UIScreen::Queue;
        return true;
      }

      break;
    }

    case UIScreen::NowPlaying:
    {
      if (e == Event::Character('1') || e == Event::Escape)
      {
        m_activeScreen = UIScreen::Main;
        return true;
      }

      if (e == Event::Character('3'))
      {
        m_activeScreen = UIScreen::Queue;
        return true;
      }

      if (e == Event::Character('l'))
      {
        if (!m_audioPtr)
          return true;

        auto meta = m_audioPtr->getCurrentMetadata();
        if (!meta)
          return true;

        auto term       = ftxui::Terminal::Size();
        int  half_width = term.dimx / 2;
        int  wrap_width = std::max(10, term.dimx - half_width - 6);

        m_nowState.fetchLyricsFromLRCAsync(*meta, wrap_width, m_threadManager);
        return true;
      }

      if (e == Event::ArrowDown)
      {
        m_nowState.setSelectedIndex(m_nowState.selectedIndex() + 1);
        return true;
      }

      if (e == Event::ArrowUp)
      {
        m_nowState.setSelectedIndex(m_nowState.selectedIndex() - 1);
        return true;
      }

      if (e == Event::Character('g')) // top
      {
        m_nowState.setSelectedIndex(0);
        return true;
      }

      if (e == Event::Character('G')) // bottom
      {
        auto& lyrics = m_nowState.lyrics();
        if (!lyrics.empty())
        {
          m_nowState.setSelectedIndex(static_cast<int>(lyrics.size()) - 1);
        }
        return true;
      }

      break;
    }

    case UIScreen::Queue:
    {
      if (e == Event::Character('1') || e == Event::Escape)
      {
        m_activeScreen = UIScreen::Main;
        return true;
      }

      if (e == Event::Character('2'))
      {
        m_activeScreen = UIScreen::NowPlaying;
        return true;
      }

      const size_t playlistSize = m_audioPtr->getPlaylistSize();

      if (e == Event::ArrowDown)
      {
        m_queueState.moveSelection(1, playlistSize);
        return true;
      }

      if (e == Event::ArrowUp)
      {
        m_queueState.moveSelection(-1, playlistSize);
        return true;
      }

      if (e == Event::Character('d'))
      {
        if (playlistSize > 0)
        {
          const auto idx = static_cast<size_t>(m_queueState.selected());

          m_audioPtr->removeFromPlaylist(idx);

          const size_t newSize = m_audioPtr->getPlaylistSize();
          m_queueState.onItemRemoved(newSize);

          if (m_mpris)
          {
            m_mpris->updateMetadata();
            m_mpris->notify();
          }
        }
        return true;
      }

      if (e == Event::Character('g')) // top
      {
        m_queueState.moveSelection(0, playlistSize);
        return true;
      }

      if (e == Event::Character('G')) // bottom
      {
        if (playlistSize > 0)
        {
          m_queueState.moveSelection(static_cast<int>(playlistSize) - 1, playlistSize);
        }
        return true;
      }

      break;
    }

    default:
      break;
  }

  // Global audio controls

  if (e == Event::Character('p'))
  {
    m_audioPtr->isPlaying() ? m_audioPtr->pauseCurrent() : m_audioPtr->playCurrent();
    return true;
  }

  if (e == Event::Character('n'))
  {
    m_threadManager.executeWithTelemetry([&](audio::Service& audio) -> void { audio.nextTrack(); });
    m_mpris->updateMetadata();
    m_mpris->notify();
    return true;
  }

  if (e == Event::Character('x'))
  {
    m_threadManager.executeWithTelemetry([&](audio::Service& audio) -> void
                                         { audio.randomTrack(); });
    m_mpris->updateMetadata();
    m_mpris->notify();
    return true;
  }

  if (e == Event::Character('b'))
  {
    m_threadManager.executeWithTelemetry([&](audio::Service& audio) -> void
                                         { audio.previousTrack(); });
    m_mpris->updateMetadata();
    m_mpris->notify();
    return true;
  }

  if (e == Event::Character('r'))
  {
    m_audioPtr->restartCurrent();
    return true;
  }

  if (e == Event::Character('j'))
  {
    m_threadManager.requestSeek(m_threadManager.getPendingSeek() - 2);
    return true;
  }

  if (e == Event::Character('k'))
  {
    m_threadManager.requestSeek(m_threadManager.getPendingSeek() + 2);
    return true;
  }

  if (e == Event::Character('='))
  {
    adjustVolume(+0.05f);
    return true;
  }

  if (e == Event::Character('-'))
  {
    adjustVolume(-0.05f);
    return true;
  }

  if (e == Event::Character('m'))
  {
    toggleMute();
    return true;
  }

  if (e == Event::Character('a'))
  {
    // enqueue all songs by selected artist
    if (m_libraryState.focusOnArtists())
    {
      if (m_libraryState.selected_artist < 0 ||
          m_libraryState.selected_artist >= (int)m_libraryState.artists.size())
        return true;

      const auto& artist = m_libraryState.artists[m_libraryState.selected_artist];

      query::songmap::read::forEachSong(*m_songMap,
                                        [&](const Artist& a, const Album&, const Disc&,
                                            const Track&, const ino_t,
                                            const std::shared_ptr<Song>& song) -> void
                                        {
                                          if (a != artist)
                                            return;

                                          auto handle = m_audioPtr->registerTrack(song);
                                          m_audioPtr->addToPlaylist(handle);
                                        });

      if (m_mpris)
      {
        m_mpris->updateMetadata();
        m_mpris->notify();
      }

      return true;
    }

    // else add selected song only
    if (m_libraryState.selected_album_index >= 0 &&
        m_libraryState.selected_album_index < (int)m_libraryState.view_song_objects.size())
    {
      auto songObj = m_libraryState.view_song_objects[m_libraryState.selected_album_index];

      if (songObj)
      {
        auto handle = m_audioPtr->registerTrack(songObj);
        m_audioPtr->addToPlaylist(handle);

        if (m_mpris)
        {
          m_mpris->updateMetadata();
          m_mpris->notify();
        }
      }
    }

    return true;
  }

  return false;
}

void EventHandler::playSelected()
{
  m_audioPtr->clearPlaylist();

  if (m_libraryState.selected_album_index < 0 ||
      m_libraryState.selected_album_index >= (int)m_libraryState.view_song_objects.size())
    return;

  auto songObj = m_libraryState.view_song_objects[m_libraryState.selected_album_index];

  if (!songObj)
    return;

  auto handle = m_audioPtr->registerTrack(songObj);

  m_audioPtr->addToPlaylist(handle);
  m_threadManager.executeWithTelemetry([&](audio::Service& audio) -> void { audio.nextTrack(); });

  query::songmap::read::forEachSongInAlbum(
    *m_songMap, m_audioPtr->getCurrentMetadata()->artist, m_audioPtr->getCurrentMetadata()->album,
    [&](const Disc&, const Track&, const ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (song->metadata.track <= m_audioPtr->getCurrentMetadata()->track)
        return;

      auto h = m_audioPtr->registerTrack(song);
      m_audioPtr->addToPlaylist(h);
    });

  m_mpris->updateMetadata();
  m_mpris->notify();
}

} // namespace frontend::tui::ui
