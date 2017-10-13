# Scroll Wheel Fix for VB6 (and others!)

This is a small program that will make your mouse's scroll wheel work in various older programs - mainly Visual Basic 6's code window. 
I noticed a lot of people coming to my old personal site from Google looking for help with their mouse wheel. So, I decided to fix it.

Most of this was written in 2004, so it's probably awful. Sorry.

## Installation:
Just unzip the files somewhere convenient for you. That's all! Delete them to uninstall.

## Usage
Start up "VB6ScrollWheelFix.exe", and then use your mouse wheel in VB6. That's all! The program puts an icon in your task bar that looks like a red mouse. When you want to quit the program, right click the icon and choose "Quit".

It is possible to support many programs by adjusting its INI file. The included INI file includes support for:

* VB6 Code Window
* VB6 Resource Design Window
* SourceSafe 6 Diff Viewer
* Windiff
* VBA Editor
It also supports horizontal scrolling if your mouse supports it, or by holding CTRL while scrolling vertically.

The scroll speed for each application can be adjusted by modifying the INI file. It should be fairly self explanitory: just edit the values for VertMsgCount and HorzMsgCount for your program's settings. Fractional values like 1.6 and negative values work just fine! Keep in mind that the program simulates clicking the arrows on a scrollbar, so entering a value of 0.1 will not give you more fine grained control. Instead you'll just have to scroll more to get 1 scroll message sent. But newer mice may work better with the fractional values.
