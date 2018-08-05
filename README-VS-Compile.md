# Compiling using Microsoft Visual Studio

## First, `p9813.dll`

After some poking, I was able to get the `p9813.dll` to compile. I had to drop `CDM v2.12.28 WHQL Certified.zip` from (http://www.ftdichip.com/Drivers/D2XX.htm)[http://www.ftdichip.com/Drivers/D2XX.htm] into the root of this repo and add a replacement for the `gettimeofday()` call (which I got from here(https://gist.github.com/ugovaretto/5875385)[https://gist.github.com/ugovaretto/5875385]).

*Note: If you use this, you'll probably have to do some leg work to get all of the `.h`, and `.lib` files from FTDI's zip file in the right place.*

## Second, `TotalControl.dll`

More poking and I've gotten `TotalControl.dll` to compile as well as all of the Java nonesense.

I've removed some things from the gitignore because I really do not want to do all of this mess again later. Sorry.

## Third, `it-does-not-work.dll`

Yeah, after all of this, Processing is yelling about not having the native code it needs. I'm moving on...