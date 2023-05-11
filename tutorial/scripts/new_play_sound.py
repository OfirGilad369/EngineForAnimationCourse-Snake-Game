import vlc
import time
import os
import sys


def play_sound(audio_file, audio_volume):
    sounds_lengths = {
        "bonus_object.mp3": 3,
        "game_over.mp3": 8,
        "gold_object.mp3": 2,
        "health_object.mp3": 3,
        "obstacle_object.mp3": 1,
        "score_object.mp3": 3,
        "stage_complete.mp3": 12,
        "shield.mp3": 5
    }

    media_player = vlc.MediaPlayer(audio_file)
    media_player.audio_set_volume(int(float(audio_volume)))
    media_player.play()
    time.sleep(sounds_lengths[audio_file] + 1)
    media_player.stop()


if __name__ == "__main__":
    audio_file = sys.argv[1]
    audio_volume = sys.argv[2] # Volume must be integer (0-100)
    # Current Directory: ./EngineForAnimationCourse-FinalProject/build
    os.chdir('../tutorial/sounds')

    play_sound(audio_file, audio_volume)
