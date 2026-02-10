#include "PlaylistController.h"

#include <QRandomGenerator>
#include <algorithm>

#include "samples.h"      // Sampler
#include "ConfigModel.h"  // playlist storage we will add

PlaylistController::PlaylistController(Sampler* sampler, ConfigModel* model, QObject* parent)
    : QObject(parent), m_sampler(sampler), m_model(model)
{
    // When a track ends, Sampler emits onStopPlaying()
    connect(m_sampler, &Sampler::onStopPlaying, this, &PlaylistController::onSamplerStopped, Qt::QueuedConnection);

    m_shuffle = m_model->getPlaylistShuffle();
    m_repeat = (RepeatMode)m_model->getPlaylistRepeatMode();

    rebuildOrder();
}

QVector<SoundInfo> PlaylistController::getPlaylist() const
{
    return m_model->getPlaylist();
}

void PlaylistController::setPlaylist(const QVector<SoundInfo>& items)
{
    m_model->setPlaylist(items);
    emit playlistChanged();
    rebuildOrder();
}

void PlaylistController::addFiles(const QStringList& paths)
{
    QVector<SoundInfo> items = getPlaylist();
    for (const auto& p : paths)
    {
        SoundInfo s;
        s.filename = p;
        items.push_back(s);
    }
    setPlaylist(items);
}

void PlaylistController::setShuffle(bool enabled)
{
    m_shuffle = enabled;
    m_model->setPlaylistShuffle(enabled);
    rebuildOrder();
}

void PlaylistController::setRepeatMode(RepeatMode mode)
{
    m_repeat = mode;
    m_model->setPlaylistRepeatMode((int)mode);
}

void PlaylistController::rebuildOrder()
{
    const int n = getPlaylist().size();
    m_order.clear();
    m_order.reserve(n);

    for (int i = 0; i < n; ++i)
        m_order.push_back(i);

    if (m_shuffle && n > 1)
    {
        // Fisher-Yates shuffle
        for (int i = n - 1; i > 0; --i)
        {
            int j = (int)QRandomGenerator::global()->bounded(i + 1);
            std::swap(m_order[i], m_order[j]);
        }
    }

    if (m_currentIndex >= 0 && m_currentIndex < n)
        m_orderPos = m_order.indexOf(m_currentIndex);
    else
        m_orderPos = -1;
}

void PlaylistController::play(int startIndex)
{
    const auto items = getPlaylist();
    if (items.isEmpty())
        return;

    if (startIndex < 0) startIndex = 0;
    if (startIndex >= items.size()) startIndex = items.size() - 1;

    rebuildOrder();

    m_playingPlaylist = true;

    if (m_shuffle)
    {
        m_orderPos = 0;
        m_currentIndex = m_order[m_orderPos];
    }
    else
    {
        m_currentIndex = startIndex;
        m_orderPos = m_order.indexOf(m_currentIndex);
    }

    emit nowPlayingChanged(m_currentIndex);
    playIndex(m_currentIndex);
}

void PlaylistController::playSelected(int index)
{
    play(index);
}

void PlaylistController::stop()
{
    m_playingPlaylist = false;
    m_currentIndex = -1;
    m_orderPos = -1;
    m_sampler->stopPlayback();
    emit nowPlayingChanged(-1);
}

int PlaylistController::nextIndexAfterStop()
{
    const int n = getPlaylist().size();
    if (n == 0) return -1;

    if (m_repeat == RepeatMode::One && m_currentIndex >= 0)
        return m_currentIndex;

    if (m_shuffle)
    {
        if (m_orderPos < 0) m_orderPos = 0;
        else m_orderPos++;

        if (m_orderPos >= m_order.size())
        {
            if (m_repeat == RepeatMode::All)
            {
                rebuildOrder();
                m_orderPos = 0;
            }
            else
            {
                return -1;
            }
        }
        return m_order[m_orderPos];
    }
    else
    {
        int next = m_currentIndex + 1;
        if (next >= n)
        {
            if (m_repeat == RepeatMode::All) next = 0;
            else return -1;
        }
        return next;
    }
}

void PlaylistController::next()
{
    if (!m_playingPlaylist) return;

    int ni = nextIndexAfterStop();
    if (ni < 0) { stop(); return; }

    m_currentIndex = ni;
    emit nowPlayingChanged(m_currentIndex);
    playIndex(m_currentIndex);
}

void PlaylistController::prev()
{
    if (!m_playingPlaylist) return;

    const int n = getPlaylist().size();
    if (n == 0) return;

    if (m_shuffle)
    {
        if (m_orderPos > 0) m_orderPos--;
        m_currentIndex = m_order[m_orderPos];
    }
    else
    {
        m_currentIndex = std::max(0, m_currentIndex - 1);
        m_orderPos = m_order.indexOf(m_currentIndex);
    }

    emit nowPlayingChanged(m_currentIndex);
    playIndex(m_currentIndex);
}

void PlaylistController::onSamplerStopped()
{
    if (!m_playingPlaylist) return;
    if (m_ignoreNextStop)
    {
        m_ignoreNextStop = false;
        return;
    }

    int ni = nextIndexAfterStop();
    if (ni < 0)
    {
        stop();
        return;
    }

    m_currentIndex = ni;
    emit nowPlayingChanged(m_currentIndex);
    playIndex(m_currentIndex);
}

void PlaylistController::playIndex(int index)
{
    const auto items = getPlaylist();
    if (index < 0 || index >= items.size())
        return;

    // playFile() stops the current sound and emits onStopPlaying; ignore that one.
    m_ignoreNextStop = true;
    bool ok = m_sampler->playFile(items[index]);
    if (!ok)
    {
        m_ignoreNextStop = false;
        // Failed to open file; stop playlist to avoid loops.
        stop();
    }
}
