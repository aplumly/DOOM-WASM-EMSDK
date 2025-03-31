# DOOM WebAssembly Port

This is a WebAssembly port of the classic game DOOM, built using Emscripten (emsdk). Achieved by modifying [https://github.com/id-Software/DOOM.git](https://github.com/id-Software/DOOM.git). Completely reworked `i_video.c` to use SDL/SDL2 as X11 is not compatible with emsdk. Small definition changes were made to allow for compilation. Reworked the `D_DoomLoop()` to use `emscripten_set_main_loop()`.

---

## Environment Setup (Debian)

### Step 1: Clone emsdk

Clone the emsdk repository:

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
```

### Step 2: Install and Activate Latest Emscripten

Install and activate the latest emsdk version:

```bash
./emsdk install latest
./emsdk activate latest
```

After running these commands, you'll receive instructions to set up your environment variables.

### Step 3: Configure Path

Configure the environment variables by executing the command provided in the output, typically:

```bash
source ./emsdk_env.sh
```

Make sure you run this command every time you open a new terminal or add it to your `.bashrc` or `.profile`.

### Step 4: Install Dependencies

Make sure you have `make` installed:

```bash
sudo apt update
sudo apt install build-essential
```

Additionally, ensure you have `node` and `npm` installed:

```bash
sudo apt install nodejs npm
```

---

## Building and Running the Project

### Step 1: Project Structure

Create your project directory named `doomsource` and include a subdirectory named `wad` containing your WAD files:

```
doomsource/
└── wad/
    └── [your WAD files here]
```

### Step 2: Build the Project

Navigate to your project directory and build using:

```bash
make
```

Compiled files (`doom.wasm`, `doom.js`, `doom.data`) will be generated in the `build` directory and copied to your testing directory automatically.

### Step 3: Running the Server

Navigate to your testing directory and install dependencies:

```bash
cd ../testing
npm install
```

Then run the Node.js server:

```bash
node server.js
```

Now you can access your DOOM WebAssembly game from your browser at `http://localhost:8000`.

---

## TODO

- Get sounds working.
  - &#x20;may just need to add the sndserver to the .data file and set the path correctly in the defines. sndserver will have to be compiled. It may be incompatible with emsdk.
  - [https://github.com/id-Software/DOOM.git](https://github.com/id-Software/DOOM.git) source for sndserver in the original linux source.
  - If this fails, the sound code will have to be rewritten to use sdl/sdl2
- Improve the wad loading/swapping experience. The current solution is hacky.&#x20;
  - the best solution, if possible is to preserve the original code that does this so that the wad files can be swapped in your /wad folder before building .data. 
- Get networking working for possible multiplayer
  - To get started with this, head to [https://github.com/id-Software/DOOM.git](https://github.com/id-Software/DOOM.git) and grab the networking source material.

