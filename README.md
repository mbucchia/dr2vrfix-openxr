# dr2vrfix-openxr: A wrapper of the Dirt Rally 2 VR eye accomodation fix to work with the WMR OpenXR runtime.

This project allows the use of the [Dirt Rally 2 VR eye accomodation fix v2.0](https://www.kegetys.fi/dirt-rally-2-vr-eye-accomodation-fix-v2-0/) with [OpenComposite](https://gitlab.com/znixian/OpenOVR) and the WMR OpenXR runtime.

Download from [here](https://github.com/mbucchia/dr2vrfix-openxr/releases/download/0.1/dr2vrfix-openxr.zip).

Special thanks to Guus for testing the fix with OpenComposite and OpenXR Toolkit on HP Reverb G2!

DISCLAIMER: This software is distributed as-is, without any warranties or conditions of any kind. Use at your own risks.

## How does it work?

The project creates a `d3d11.dll` that wraps the previous implementation of the Dirt Rally 2 VR eye accomodation fix (aka "dr2vrfix"). The dr2vrfix implements an old `d3d11.dll` interface that is missing certain symbols needed by some OpenXR runtimes, like the WMR OpenXR runtime. The WMR OpenXR runtime needs newer symbols such as `D3D11On12CreateDevice()`. The dr2vrfix only needs to hook the `D3D11CreateDevice()` function. This project's wrapper `d3d11.dll` forwards the calls to `D3D11CreateDevice()` to dr2vrfix (which we rename to `dr2vrfix.dll`) and implements a bridge for all other symbols needed by the WMR OpenXR runtime, such as `D3D11On12CreateDevice()`.
