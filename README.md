# Grow_Light_Controller
Grow Light Controller for Arduino

## There are 4 main goals here with some constraints
### Goals:
1. to run a high voltage light array controller by an atmega238 P microcontroller 
2. Read rotary encoder and translate signals into useable data
3. Parse user input into a programable light-schedule 
4. Display program data on a FC16_HW  8 x 8 x 4 LED matrix display 

### Constraints:
1. Flash Memory: 
    1. the atmega328p has 32KB this is the maximum prgram storage space
2. SRAM Stack and Heap size for dynamic program memory
    1. this varies and is HIGHLY dependent on state management within the program
    2. de-allocated memory can be 'lost' in the heap, increasing the heap size until a collision occurs between stack an heap. three things have to be carefully managed
        1. stack
        2. heap
        3. maintenance of 'free' memory or the unallocated memory 'between' the heap and stack

3. **Simplicity in readablility and debugging**:
    1. This program is only a part of a larger design that will have a significant amount of places where issues can occur. Debugging the physical circuit is challenging enough. So The code needs to be as plain and readable as possible in order to facilitate a working platform.
