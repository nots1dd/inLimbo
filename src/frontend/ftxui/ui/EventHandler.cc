#include "frontend/ftxui/ui/EventHandler.hpp"
#include "query/SongMap.hpp"

using namespace ftxui;

namespace frontend::tui::ui
{

EventHandler::EventHandler(UIScreen& activeScreen, state::library::LibraryState& libraryState,
                           state::now_playing::NowPlayingState& nowState,
                           managers::ThreadManager&             threadManager,
                           threads::SafeMap<SongMap>* songMap, mpris::Service* mpris)
    : m_activeScreen(activeScreen), m_libraryState(libraryState), m_nowState(nowState),
      m_threadManager(threadManager), m_songMap(songMap), m_mpris(mpris)
{
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

      break;
    }

    case UIScreen::NowPlaying:
    {
      if (e == Event::Character('1') || e == Event::Escape)
      {
        m_activeScreen = UIScreen::Main;
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
    m_audioPtr->setVolume(std::min(1.5f, m_audioPtr->getVolume() + 0.05f));
    return true;
  }

  if (e == Event::Character('-'))
  {
    m_audioPtr->setVolume(std::max(0.0f, m_audioPtr->getVolume() - 0.05f));
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
