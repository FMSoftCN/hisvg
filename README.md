# hiSVG

- [Introduction](#introduction)
- [Dependencies](#dependencies)
- [Building](#building)
- [Usage](#usage)
- [Copying](#copying)

## Introduction

hiSVG is a high performance SVG rendering library. It is derived form librsvg of GNOME Project.

We have made the following changes :

* We changed prefix from rsvg to hisvg.
* We replaced Pango by MiniGUI. It is used for layout svg text node.
* We replaced libcroco by hiDomLayout. It is used for parse css and layout svg node.
* We replaced Cairo by hiCairo. hiCairo is derived form Cairo.
* We removed gdk-pixbuf support.


## Dependencies

hiSVG depends on the following libraries:

- [GLib](https://developer.gnome.org/glib/stable/)
- [libXML2](http://www.xmlsoft.org/)
- [MiniGUI](https://gitlab.fmsoft.cn/VincentWei/minigui/)
- [hiCairo](https://gitlab.fmsoft.cn/hybridos/hicairo/)
- [hiDomLayout](https://gitlab.fmsoft.cn/hybridos/hidomlayout/)


## Building

We recommend that you use a latest Linux distribution with long term support,
for example, Ubuntu Linux LTS 18.04 or 20.04.

On Ubuntu Linux LTS 18.04 or 20.04, you should run `apt install <package_name>`
to install the following packages:

 * Building tools:
    * cmake
 * Dependent libraries (all are optional):
    * libglib2.0-dev
    * libxml2-dev

Then build MiniGUI, hiCairo and hiDOMLayout library. (Please follow the README files of these projects).

hiSVG uses cmake to configure and build the project.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make install
```

## Usage

* create HiSVGHandle

```
HiSVGHandle* hisvg_handle_new_from_file (const gchar* file_name, GError** error);
```

* create surface

```
cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
```

* render svg

```
gboolean hisvg_handle_render_cairo (HiSVGHandle* handle, cairo_t* cr, const HiSVGRect* viewport, const char* id, GError** error);
```



## Copying

Copyright (C) 2020 FMSoft <https://www.fmsoft.cn>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General License for more details.

You should have received a copy of the GNU Lesser General License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

### Commercial License

If you cannot accept LGPLv3, you need to be licensed from FMSoft.

For more information about the commercial license, please refer to
<https://hybridos.fmsoft.cn/blog/hybridos-licensing-policy>.

### Special Statement

The above open source or free software license(s) does
not apply to any entity in the Exception List published by
Beijing FMSoft Technologies Co., Ltd.

If you are or the entity you represent is listed in the Exception List,
the above open source or free software license does not apply to you
or the entity you represent. Regardless of the purpose, you should not
use the software in any way whatsoever, including but not limited to
downloading, viewing, copying, distributing, compiling, and running.
If you have already downloaded it, you MUST destroy all of its copies.

The Exception List is published by FMSoft and may be updated
from time to time. For more information, please see
<https://www.fmsoft.cn/exception-list>.

