--Polski-- \
Wersja Visual Studio - 2022 \
Wersja SFML - 3.0.2 x64 

Przed uruchomieniem należy we właściwościach projektu w Visual Studio dla platformy x64: \
-Dla All Configurations - C/C++ - Additional Include Directories - dodać ścieżkę do folderu include projektu \
-Dla All Configurations - Linker - Additional Library Directories - dodać ścieżkę do folderu lib projektu \
-Dla Release - Input - Additional Dependecies - dodać: \
sfml-system.lib \
sfml-graphics.lib \
sfml-window.lib \
sfml-audio.lib \
sfml-network.lib \
-Dla Debug - Input - Additional Dependecies - dodać: \
sfml-system-d.lib \
sfml-graphics-d.lib \
sfml-window-d.lib \
sfml-audio-d.lib \
sfml-network-d.lib

--English-- \
Visual Studio Version - 2022 \
SFML Version - 3.0.2 x64

Prior to running the program, user should, in Visual Studio project properties for x64 platform: \
-For All Configurations - C/C++ - Additional Include Directories - add the path to the project's include directory \
-For All Configurations - Linker - Additional Library Directories - add the path to the project's lib directory \
-For Release - Input - Additional Dependecies - add: \
sfml-system.lib \
sfml-graphics.lib \
sfml-window.lib \
sfml-audio.lib \
sfml-network.lib \
-For Debug - Input - Additional Dependecies - add: \
sfml-system-d.lib \
sfml-graphics-d.lib \
sfml-window-d.lib
sfml-audio-d.lib
sfml-network-d.lib
