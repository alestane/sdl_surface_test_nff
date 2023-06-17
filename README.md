### Minimum repro case for issue converting 1-depth bitmap in memory to SDL texture or surface

Left window uses low-level algorithm to translate bits to pixels and serves as reference case
Middle window blits 1-bit-depth bitmap to window by converting the surface to a temporary one.
Right window generates a texture from the surface wrapped around the underlying bitmap and sends it to the window renderer with a rotation.
