function showScreen(id) {
  document.querySelectorAll('.screen').forEach(s => {
    s.classList.remove('active');
  });

  document.body.classList.toggle(
    "now-playing-active",
    id === "nowPlaying"
  );

  if (id === 'library')
    document.getElementById('libraryScreen').classList.add('active');
  else
    document.getElementById(id).classList.add('active');

  // One-time load when entering Now Playing
  if (id === "nowPlaying") {
    loadNowPlaying();
  }
}

async function waitForBackend() {
  while (
    typeof window.__getArtists !== "function" ||
    typeof window.__getAlbumsForArtist !== "function"
  ) {
    await new Promise(r => setTimeout(r, 10));
  }
}

async function loadArtists() {
  await waitForBackend();

  const artists = await window.__getArtists();

  const library = document.getElementById("library");
  library.innerHTML = "";

  for (const artist of artists) {
    const div = document.createElement("div");
    div.className = "item";
    div.textContent = artist;
    div.onclick = () => loadArtist(artist);
    library.appendChild(div);
  }
}

async function loadArtist(name) {
  document.getElementById("songTitle").innerText = "Browsing";
  document.getElementById("songArtist").innerText = name;

  await waitForBackend();

  const albums = await window.__getAlbumsForArtist(name);

  const title = document.getElementById("libraryTitle");
  title.textContent = name;

  const albumView = document.getElementById("albumView");
  albumView.innerHTML = "";

  for (const album of albums) {
    const h = document.createElement("h3");
    h.textContent = album.album;
    albumView.appendChild(h);

    for (const disc of album.discs) {
      const d = document.createElement("div");
      d.className = "disc";
      d.textContent = `Disc ${disc.disc}`;
      albumView.appendChild(d);

      for (const track of disc.tracks) {
        const row = document.createElement("div");
        row.className = "track";
        row.textContent =
          `${track.track.toString().padStart(2, "0")}  ${track.title}`;

          row.onclick = async () => {
            try {
              await window.__playTrack(track.title);
              await onTrackChanged();
            } catch (e) {
              console.error("Play failed:", e);
            }
          };

        albumView.appendChild(row);
      }
    }
  }

  showScreen("library");
}

async function onTrackChanged() {
  // Update Now Playing screen (if visible)
  await loadNowPlaying();

  // Update footer immediately
  const meta = await window.__fetchMetadata?.();
  if (meta) {
    document.getElementById("songTitle").innerText = meta.title || "—";
    document.getElementById("songArtist").innerText = meta.album || "—";
    console.log(meta);
  }
}

async function nextTrack() {
  if (typeof window.__nextTrack !== "function") return;

  try {
    await window.__nextTrack();
  } catch (e) {
    // Ignore backend result completely
    console.warn("Next track backend response ignored:", e);
  } finally {
    await onTrackChanged();
  }
}

async function prevTrack() {
  if (typeof window.__prevTrack !== "function") return;

  try {
    await window.__prevTrack();
  } catch (e) {
    // Ignore backend result completely
    console.warn("Prev track backend response ignored:", e);
  } finally {
    await onTrackChanged();
  }
}

async function playPause() {
  if (typeof window.__playPauseTrack !== "function") return;

  try {
    await window.__playPauseTrack();
  } catch (e) {
    // Ignore backend result completely
    console.warn("Play pause track backend response ignored:", e);
  } finally {
    await onTrackChanged();
  }
}

async function loadNowPlaying() {
  if (typeof window.__fetchMetadata !== "function")
    return;

  try {
    const meta = await window.__fetchMetadata();
    if (!meta) return;

    document.getElementById("npTitle").innerText = meta.title || "—";
    document.getElementById("npAlbum").innerText = meta.album || "—";
    document.getElementById("npGenre").innerText = meta.genre || "—";

    const art = document.getElementById("npArt");
    if (meta.artUrl) {
      if (art.src !== meta.artUrl) {
        art.src = meta.artUrl;
      }
      art.style.display = "block";
    } else {
      art.style.display = "none";
    }
  } catch (e) {
    console.error("Failed to load metadata:", e);
  }
}

document.addEventListener("DOMContentLoaded", () => {
  loadArtists().catch(err => console.error(err));
});

async function quitBackend() {
  if (typeof window.__quitApp === "function") {
    try {
      await window.__quitApp();
    } catch (e) {
      // Ignore — app is quitting anyway
      console.warn("Quit backend failed:", e);
    }
  }
}

/* Fired when window is about to close */
window.addEventListener("beforeunload", (e) => {
  quitBackend();
});

window.addEventListener("pagehide", () => {
  quitBackend();
});
