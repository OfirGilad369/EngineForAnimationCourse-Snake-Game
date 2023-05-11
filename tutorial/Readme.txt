EngineForAnimationCourse-FinalProject - Snake 3D


Game Description:
The game itself is based on a stages system, and there are 3 levels in total that must be completed to finish the game. 
The purpose of the player is to collect as many score points as possibly without dying and achieve new highest scores. 
Dying is possible if the snake: hits itself, hits the background of the stage, collides with a moving obstacle, or by the Stage Timer damage every minute.
Each level is unique in several aspects and that includes: The amount of game objects that are present at the same time, their respawn time, duration time and their effects (for the best and for the worst) 
(For example: Gold objects will give more gold, but Obstacle objects will deal more damage), making it a lot more challenging.


Game Features:
1. Cameras - By pressing the V or B keys there is an option to change up to 4 different view:
- Snake far view - A far view from the snake with options to: look up, down, left, right, zoom in and zoom out.
- Snake front view - A view from the head of the snake to where it looks.
- Snake back view - A view from the head of the snake to his behinds.
- Snake global static view - A view from the top of the stage to see everything with options to zoom in and zoom out.

2. Collision Detection - The snake is covered by 16 cylinders, and for each cylinder, including the game objects, I have created AABB trees. 
And by using the same logic as in assignment 2, to check if the snake is about to collide with an object I check the snake and the object AABB trees, and if a collision was found, the relevant event will get triggered.
The snake can collide with every game object in the game, including its own body.

3. Skinning - The movement of the snake is done by skinning. The logic starts by creating the 16 joints with a distance of 1.6 in the Z axis between them, stretching the snake body to the length of all the joints, and then calculating the weights for each vertex. 
On each step of the movement I calculate the new position for each joint according to the direction vector that is decided by the keyboard presses. 
The actual skinning is done by igl::dqs that is provided by the library. 
I defined the vT and vQ whereas vT is the positions of the joints and vQ is always the identity. igl::dqs gave me "U" which is a matrix that has the position of all the vertices after the dqs to update the snake body accordingly.

4. Game Objects - There are 6 different type of game objects in the game:
- Health Objects - Heal some of the snake health points as long as its health points don't equal to its max health points.
- Score Objects - Give score points to the snake which are required to complete the stage objective and unlock the exit from the stage to the next stage.
- Gold Objects - Give gold points to the snake which can be spent on the Shop to improve the snake stats.
- Bonus Objects - Give a random bonus to the snake for a short amount of time. The available bonuses are:
a. Magnet - Magnets Health, Score and Gold objects to the snake.
b. Freeze Time - Stops the Stage Timer and delays the next wave of the timer damage.
c. Shield - Blocks Obstacle objects from hitting the snake and prevents self collision.
d. Stats Boost - The effect of Health, Score and Gold objects get multiplied by 2.
- Obstacle Objects - Deal a certain amount of damage to the snake health points
- Exit Object - Appears after the game objective has been reached, and upon collision, gives the snake the option to move to the next stage.

5. Basic locomotion - The snake body moves by dual quaternions calculations as seen in the lectures. These calculations on each frame give the snake its snaky movement.


Additional Points:
1. Movement of Game Objects according to a Bezier Curve - The Bonus and Obstacle objects get a randomly generated bezier curve that defines their traversal paths for a certain amount of time. 
There are 4 points in total that are generated for each of these objects. With those 4 points we calculate the movement on the curve and display the curve in a grey line.
Whenever the object reaches the end of the curve, it goes back on the same path.

2. Shop - The game store is in the game menu and is described above, allows you to buy stuff to boost your snake's stats:
- Increase Max Health Points
- Increase Score Multiplier
- Increase Gold Multiplier
- Increase Max Movement Speed
- Increase Bonuses Duration

3. Stats - The game monitors almost every possible info about the events that occur in the game.
The stats and statistics that are being collected are:
- Snake Stats: max health, score multiplier, gold multiplier, bonuses duration, max movement speed
- Statistics: total health points healed, total health points lost, total score points earned, total gold earned, total gold spent, total bonuses collected, total deaths, time played   

4. Leaderboard system - A fully working highscore system that is based on JSON, each score upon a finished game session gets saved into a .json file, and any new record is saved and displayed at the menu. 
The high score is based on the total amount of score points that were collected from all the stages combined.

5. Music and Sounds - For almost every menu and object there are special musics and sounds. There are 17 audio files in total to give the ultimate game experience to the user.
The music and sounds volumes can get changed in the settings menu.

6. Custom Objects, Maps, and Images - The Game Objects and the Snake head and tail have a unique custom design which were created with the help of Tinkercad.
Each stage has a different map to distinguish between them and improve the gameplay experience.
And finally almost every menu has its own unique image including the loading screen, and the stage countdown.

7. Interactive Menus - The game has 13 different menus which are:
Login Menu - Enables the user to continue from it last saved record, or start a new game.
Main Menu - The main menu that enables navigation to the menus: Stage Selection Menu, Shop Menu, Stats menu, Hall Of Fame Menu, Settings Menu, Credits Menu. And also an option to save the game.
Stage Selection Menu - Enables the user to select which stage to play.
Shop Menu - The Shop as described above, which can be opened from the main menu, and between stages. 
Stats Menu - Display all the collected info about the snake stats and the game statistics.
Hall Of Fame Menu - Show the top 10 highest scores achieved in the game. 
Settings Menu - Provide options to: Enable and disable skinning, control the music and sounds volumes, change the snake colours, and show the keyboard and mouse keys.
Credits Menu - Show the info about the game creator and more. 
Stage Menu - Shows relevant info for the current stage: Current snake health points, current score points, total gold, current movement speed, active bonus, score points objective and option to switch camera views.
Pause Menu - Appears when trying to pause a stage and gives the option to navigate to the Settings menu. (The menu shows the current health and current score points)
Stage Completed Menu - Appears upon a stage completion, and provides options to: Continue to the next stage, open the shop or go back to the main menu. (The menu shows the current score points and total gold)
Stage Failed Menu - Appears upon a stage failure, and provides options to: Retry and select different stages, open the shop or go back to the main menu. (The menu shows the current score points and total gold)
New High Score Menu - Appears if after a game session was finished, a new high score was achieved. There is an option to Apply the new score or go back to the main menu without saving it.


