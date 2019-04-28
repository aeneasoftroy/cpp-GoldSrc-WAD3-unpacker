![alt text](https://github.com/aeneasoftroy/cpp-GoldSrc-WAD3-unpacker/blob/master/goldsrcwad3.png)


#### GoldSrc WAD3 unpacker

Since this has no update since 2019 it's about time someone compiled a modern binary of the WAD3_source.

WAD is the file extension for Half-Life texture packages. An acronym for Where's All the Data?, WAD files first originated in ID Software's Doom as a general purpose file archive to help facilitate game MODs. A format of the same name was later used in ID Software's Quake engine as a texture package, then consequentially adapted by Half-Life 1's GoldSrc engine. GoldSrc specifically uses version 3 of the format, sometimes referred to as WAD3. WAD files typically contain standard texture sets, custom texture sets for individual levels, sprays, HUD elements and fonts.

WAD3 can store 3 types of data (however, WAD format itself allows up 256 types, but GoldSrc uses only 3). 

Both the GoldSrc and Quake engine use the .WAD format to store and load textures. The only differences between the two formats are that each texture in a GoldSrc .WAD file has its own palette, instead of sharing from one palette used by all of the textures, an ability to load multiple .WAD files in Half-Life; Quake uses only gfx.wad, and the file header; Quake uses WAD2, while GoldSrc uses WAD3. The WAD2 (and thus, WAD3) format itself is based on the WAD format used in the Doom engine.

### The following types are supported by the WAD3 format:

#### miptex (0x44)
    Power-of-16-sized world textures with 4 mipmaps.
#### qpic (0x42)
    Simple image with any size.
#### font (0x45)
    Fixed-height font. Contains an image and font data (row, X offset and width of a character) for 256 ASCII characters.

### Should work with games like:
    Half-Life (Valve, 1998)
    Half-Life: Opposing Force (Gearbox, 1999)
    Counter-Strike (Valve, 2000)
    Team Fortress Classic (Valve, 1999)
    Gunman Chronicles (Rewolf, 2000)
    Half-Life: Blue Shift (Gearbox, 2001)
    James Bond 007: Nightfire (Gearbox, 2002)
    Ricochet (Valve, 2000)
    Deathmatch Classic (Valve, 2001)
    Day of Defeat (Valve, 2003)
    Counter-Strike Neo (Valve, Namco, 2003)
    Counter-Strike: Condition Zero (Valve, Ritual, Gearbox, Turtle Rock Studios, 2004)
    Counter-Strike Online (Valve, NEXON, 2008) 

### Original source:
    WAD3_source.zip (2001, 8 Januari)

### Recompiled by:
    Aeneas of Troy (2019, 28 april)
