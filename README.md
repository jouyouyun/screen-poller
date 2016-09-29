Screen Poller
=================


Purpose
-----------------

Poll monitor number, if not equal with above, refreash screen infos.

Because sometimes plug or unplug VGA no event emitted by kernel.


Implementation
----------------------------------

The number of monitors via calling libdrm `drmModeGetResources`

The refrash operation via calling libxrandr  `XRRGetScreenResources`


Install
-------------

**Depends:** `libx11`,  `libdrm`, `libxrandr`

`make && sudo make install`

It will autostart after user logined.
