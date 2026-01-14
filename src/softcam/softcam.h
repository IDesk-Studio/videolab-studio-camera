#pragma once


//
// Softcam Sender API
//

#define SOFTCAM_API __cdecl

extern "C"
{
    using scCamera = void*;

    /*
        Pixel format for the virtual camera.
    */
    enum scPixelFormat
    {
        SC_PIXELFORMAT_RGB24 = 0,   // 24-bit RGB (3 bytes per pixel), no alpha channel
        SC_PIXELFORMAT_ARGB32 = 1   // 32-bit ARGB (4 bytes per pixel), with alpha channel
    };

    /*
        This function creates a virtual camera instance.

        Camera applications running on the system will be able to find the
        new virtual camera through DirectShow API.

        The `width` argument and the `height` argument specify the dimension
        of the camera image. The width and the height should be a positive
        number and be a multiple of four.

        The `framerate` argument is an optional argument that specifies the
        reference framerate of the camera. The default framerate is 60.

        If the value of the `framerate` argument is 0, the interval of each
        frame is not constrained and every frame is sent immediately by the
        `scSendFrame` function. This is useful if the application has a
        real-time source stream such as actual webcams.

        If this function succeeds, it returns the handle of a new virtual
        camera instance, otherwise, it returns a null pointer.

        This function fails if another instance already exists in the system.

        The new instance created by this function should be deleted with
        the `scDeleteCamera` function when it no longer is used.
        If the caller process terminates without calling the `scDeleteCamera`
        function, the virtual camera intance and associated resources are
        deleted automatically.
    */
    scCamera    SOFTCAM_API scCreateCamera(int width, int height, float framerate = 60.0f);

    /*
        This function creates a virtual camera instance with extended options.

        This is an extended version of `scCreateCamera` that allows specifying
        the pixel format.

        The `width` argument and the `height` argument specify the dimension
        of the camera image. The width and the height should be a positive
        number and be a multiple of four.

        The `framerate` argument is an optional argument that specifies the
        reference framerate of the camera. The default framerate is 60.

        If the value of the `framerate` argument is 0, the interval of each
        frame is not constrained and every frame is sent immediately by the
        `scSendFrame` function. This is useful if the application has a
        real-time source stream such as actual webcams.

        The `format` argument specifies the pixel format of the camera image.
        SC_PIXELFORMAT_RGB24 uses 3 bytes per pixel (24-bit RGB, no alpha).
        SC_PIXELFORMAT_ARGB32 uses 4 bytes per pixel (32-bit ARGB with alpha).
        The default format is SC_PIXELFORMAT_RGB24 for backward compatibility.

        If this function succeeds, it returns the handle of a new virtual
        camera instance, otherwise, it returns a null pointer.

        This function fails if another instance already exists in the system.

        The new instance created by this function should be deleted with
        the `scDeleteCamera` function when it no longer is used.
        If the caller process terminates without calling the `scDeleteCamera`
        function, the virtual camera intance and associated resources are
        deleted automatically.
    */
    scCamera    SOFTCAM_API scCreateCameraEx(int width, int height, float framerate = 60.0f, scPixelFormat format = SC_PIXELFORMAT_RGB24);

    /*
        This function deletes the specified virtual camera instance.
    */
    void        SOFTCAM_API scDeleteCamera(scCamera camera);

    /*
        This function sends a new frame of the specified virtual camera.

        If the framerate set to the virtual camera is not zero, this
        function tries to make the timing to deliver the new image ideal
        as much as possible by sleeping for an appropriate time inside the
        function.

        If the framerate set to the virtual camera is zero, this function
        sends the new image immediately and does not control that timing.
        This is useful if the application has a real-time source stream
        such as actual webcams.
    */
    void        SOFTCAM_API scSendFrame(scCamera camera, const void* image_bits);

    /*
        This function waits until an application connects to the specified
        virtual camera.

        If the `timeout` argument is greater than 0, this function timeouts
        after the specified time if no application accesses the virtual camera.

        This function returns `true` if an application is connected to
        the virtual camera. Otherwise, it returns `false`.
    */
    bool        SOFTCAM_API scWaitForConnection(scCamera camera, float timeout = 0.0f);

    /*
        This function reports if an application is connected to the specified
        virtual camera.

        This function returns `true` if an application is connected to
        the virtual camera. Otherwise, it returns `false`.
    */
    bool        SOFTCAM_API scIsConnected(scCamera camera);
}
