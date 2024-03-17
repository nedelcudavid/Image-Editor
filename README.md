# Image Editor
## Copyright 2022 Nedelcu Andrei-David

### Description
The command division into arguments is achieved through a parser, a data structure containing parameters similar to those from the command line (argc and argv). Among other structures are the ones storing the image and the current selection.

In the main function, the program enters the main loop where commands are parsed and corresponding functions are called.

### Image I/O
The first phase of loading an image consists of reading its header. Depending on these parameters, a pixel matrix is allocated, either monochrome or with 3 color channels, of appropriate dimensions, which will be stored in the image_t structure. 
When writing an image, the magic number is calculated based on the number of color channels of the image and the chosen option between saving the file in ASCII or binary format. After writing the header, the image content is written in the desired format. The matrix storing the image is allocated in a single continuous block of memory.

### Region Selection
The coordinates of the selection margins are stored in a structure in the main function, passed as an argument to each necessary command call. To set these, within the selection function, the number and type of command parameters are checked, and if they are valid, the desired coordinates are extracted.
I/O and selection operations are implemented in the files pbmIO.c, io_ops.c, and their associated headers.

### Rotation and Cropping
These operations have in common that the image or resulting image section is stored separately. At the end of the operation, it is either placed in the selected location or becomes the new image. Arbitrary angle rotation (multiple of 90) is done by successive rotations at 90 degrees clockwise.
Associated files: crop_rot.c and its associated header.

### Applying Filters
Applying a filter is an operation divided into multiple steps, including besides checking the correctness of arguments: determining the convolution kernel (based on the desired filter), for which there is an auxiliary function, and the actual convolution of each pixel. For this, a canvas is created, similar to the crop and rotate operations, where the filtered image is calculated, and this canvas is later copied over the selected region and freed from memory.

### Exiting the Program
The program execution stops when the EXIT command is successfully executed or when the end of input data is reached (detected by the failure of the fgets call). In these events, the program forcefully exits the main loop, deallocating dynamically allocated resources (image and parsed arguments) outside of it.
