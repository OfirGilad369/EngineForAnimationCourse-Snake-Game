import pygame
import os
import sys
from threading import Thread

audio_file = ""
player = ""


def play_music():
    player.play(loops=-1)
    pygame.event.wait()


def read_next_char():
    while True:
        char = sys.stdin.read(1)
        if char:
            break
    return char


if __name__ == "__main__":
    audio_file = sys.argv[1]

    # Current Directory: ./EngineForAnimationCourse-FinalProject/build
    os.chdir('../tutorial/sounds')

    pygame.init()
    pygame.mixer.init()
    player = pygame.mixer.Sound(audio_file)

    thread = Thread(target=play_music)
    thread.start()

    reading = True
    requested_volume = ""

    while True:
        current_char = read_next_char()
        if current_char != '\n':
            requested_volume += current_char
        else:
            player.set_volume(float(requested_volume))
            requested_volume = ""

