## q5Gomoku

q5Go derivative for gomoku. Please check the original project:
https://github.com/bernds/q5Go


# Setup repo
Supported os: Linux  

Known dependencies:  
qmake  
(qmake installation on Ubuntu 22.04:  
sudo apt install qtchooser  
sudo apt install qt5-qmake  
sudo apt-get install qtbase5-dev  
sudo apt-get install qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5  
sudo apt-get install libpcre3=2:8.39-13ubuntu0.22.04.1  
sudo apt-get install libqt5svg5*  
)  

Change directory to the ./build directory of the repo, and cast the following three commands:  
qmake ../src/q5go.pro PREFIX=.  
make  
make install  

# Run the executable
Run the executable. Still from the ./build directory run:  
./q5go

# Add gtp engine
In Settings/Preferences go to Computer Go. Then choose New. Add the name and the executable path. Arguments:gtp  
Restrictions should be not ticked off. Use for analysis should be ticked off. Then OK. Then Apply and OK.

Starting the engine for the first time is somewhat problematic. File/New game and Analysis/ Play agasinst... can be useful. Eventually the added engine should be choosen from Analysis/Choose analysis engine and Analysis/Connect analysis engin should start it.


