import vlc
import time
import os
import sys


def play_sound(sound_file):
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

    p = vlc.MediaPlayer(sound_file)
    p.play()
    time.sleep(sounds_lengths[sound_file] + 1)
    p.stop()


if __name__ == "__main__":
    sound_file = sys.argv[1]
    # Current Directory: ./EngineForAnimationCourse-FinalProject/build
    os.chdir('../tutorial/sounds')

    play_sound(sound_file)
