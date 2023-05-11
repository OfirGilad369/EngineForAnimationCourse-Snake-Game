import vlc
import time
import os
import sys


def play_music(music_file):
    music_lengths = {
        "credits.mp3": 96,
        "hall_of_fame.mp3": 97,
        "main_menu.mp3": 66,
        "opening_theme.mp3": 169,
        "shop.mp3": 128,
        "stage1.mp3": 70,
        "stage2.mp3": 94,
        "stage3.mp3": 170,
        "stats.mp3": 108,
    }

    p = vlc.MediaPlayer(music_file)
    p.play()
    time.sleep(music_lengths[music_file])
    p.stop()


if __name__ == "__main__":
    music_file = sys.argv[1]
    # Current Directory: ./EngineForAnimationCourse-FinalProject/build
    os.chdir('../tutorial/sounds')

    while(True):
        play_music(music_file)
