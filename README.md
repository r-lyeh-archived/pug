<img align="right" src="https://raw.github.com/r-lyeh/pug/master/images/logo.png" width="320" alt="@todo, disclaimer here: unknown author, pending credits http://i.ytimg.com/vi/pzPxhaYQQK8/hqdefault.jpg" />

### PUG

- pug, png with a twist: lossy image format.
- pug, `PUG = color.JPG + alpha.PNG + footer`
- pug file format, tools and related source code is all public domain.

#### Features
- PUG images are smaller than PNG, and perceptually similar to JPG.
- PUG manipulation does not require additional image libraries.
- Regular image viewers should display PUG files out of the box.

#### Cons
- PUG is a lossy file format. Reconstructed images from PUG files will not match original images.
- Regular image viewers will ignore PUG alpha channel until fully supported.

#### PUG file format specification
- RGBA32 input image is split into RGB24.jpg and A8.png images, then glued together with a footer.

```c++
RGBA32.pug = [RGB24.jpg] + [optional padding] + [A8.png] + [optional padding] + [footer]

- [?? bytes]  lossy color plane (JPG stream)
- [?? bytes]  lossy color padding (optional)
- [?? bytes]  lossless alpha plane (PNG stream)
- [?? bytes]  lossless alpha padding (optional)
- [ 4 bytes]  footer chunk - sizeof( color_plane + color_padding ) (little endian)
- [ 4 bytes]  footer chunk - sizeof( alpha_plane + alpha_padding ) (little endian)
- [ 4 bytes]  footer chunk - header plus version: literal "pug1"
```

#### Pug = lossy color + lossless alpha layers

| Original image    | Lossy color | Lossless alpha | Pug image |
| :-------------: |:-------------:| :-----:| :-----: |
| ![image](https://raw.github.com/r-lyeh/pug/master/images/panda.png) | ![image](https://raw.github.com/r-lyeh/pug/master/images/panda.pug.color.jpg) | ![image](https://raw.github.com/r-lyeh/pug/master/images/panda.pug.alpha.png) | ![image](https://raw.github.com/r-lyeh/pug/master/images/panda.pug.rebuilt.png) |
| PNG Image, 56 KiB | JPG plane, Q=75% | PNG plane, 8-bit | PUG Image, 13 KiB |
| ![image](https://raw.github.com/r-lyeh/pug/master/images/dices.png) | ![image](https://raw.github.com/r-lyeh/pug/master/images/dices.pug.color.jpg) | ![image](https://raw.github.com/r-lyeh/pug/master/images/dices.pug.alpha.png) | ![image](https://raw.github.com/r-lyeh/pug/master/images/dices.pug.rebuilt.png) |
| PNG Image, 213 KiB | JPG plane, Q=75% | PNG plane, 8-bit | PUG Image, 73 KiB |
| ![image](https://raw.github.com/r-lyeh/pug/master/images/babytux.png) | ![image](https://raw.github.com/r-lyeh/pug/master/images/babytux.pug.color.jpg) | ![image](https://raw.github.com/r-lyeh/pug/master/images/babytux.pug.alpha.png) | ![image](https://raw.github.com/r-lyeh/pug/master/images/babytux.pug.rebuilt.png) |
| PNG Image, 40 KiB | JPG plane, Q=75% | PNG plane, 8-bit | PUG Image, 20 KiB |

#### Required pixel algebra
Assuming pixel algebra is present and pixel is a vec4f, required pixel operations are:

```c++
// pug encoding: split planes: rgba into rgb+a
   rgb.at(w,h) = rgba.at(w,h) * pixel(255,255,255,0);
     a.at(w,h) = rgba.at(w,h) * pixel(0,0,0,255);

// pug decoding: join planes: rgb+a into rgba
   rgba.at(w,h) = rgb.at(w,h) * pixel(255,255,255,0) + a.at(w,h) * pixel(0,0,0,255);
```

#### Appendix: provided tools
- [pugify.exe](https://raw.github.com/r-lyeh/pug/master/tools/pugify.exe), which does PNG <=> PUG conversions ([source code](tools/pugify.cc))

Building:
```
cl pugify.cc  -I deps deps\spot\spot*.c* /Ox /Oy /MT /DNDEBUG

pugify.exe panda.png panda.pug 75
pugify.exe panda.pug panda.rebuilt.png
start panda.rebuilt.png
```

#### Appendix: notes
- Pugify uses tiny image encoders that may lead to sub-optimal JPG/PNG file sizes. Therefore, Pugify may generate sub-optimal (but fully compliant) PUG files. If you plan to use Pugify in production scenarios consider integrating better image encoders before merging RGB24 and A8 planes both together.
- [panda.png](images/panda.png) taken from http://tinypng.com website.
- [dices.png](images/dices.png) taken from POV-Ray source code (CCASA30U license), [source](http://upload.wikimedia.org/wikipedia/commons/4/47/PNG_transparency_demonstration_1.png)
- [babytux.png](images/babytux.png) by Fizyplankton (public domain), [source](http://www.minecraftwiki.net/images/8/85/Fizyplankton.png)
- Uncredited pug image logo (source? help!)

