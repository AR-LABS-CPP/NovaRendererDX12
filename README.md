# NOTE
I've decided to continue my low-level graphics programming journey with Vulkan instead. This decision comes from the fact that there just aren’t many resources for DX12, and even the ones that do exist tend to miss some important concepts. Vulkan, on the other hand, has an abundance of learning materials—both official resources from Khronos and plenty of YouTube series, articles, and code examples. Right now, learning and following along with Vulkan just feels way easier than going with DX12.

# Nova Renderer (DirectX 12)

Welcome to the Nova Renderer (DX12) repository. This is a personal project I’m working on while learning DirectX 12. After spending time with OpenGL, I decided to dive into the low-level world of DX12 and see if I can wrap my head around it, maybe even create something useful along the way.

There aren’t many beginner-friendly resources for learning DX12, so this is me working through the challenges. Hopefully, this journey will help others who are struggling with the same tech. In the future, I might write a guide to help people get started, but for now, I’m focused on building and learning at a steady pace.

## What's Done So Far

Here’s a quick checklist of what I’ve implemented so far:

- [x] **Created helper and error classes**
- [x] **Created device creation class**
- [x] **Created fence and swapchain classes**
- [x] **Created window class**
- [x] **Created command list and command queue classes**
- [x] **Created descriptor heap class**

## Resources for Learning DX12

I know how tough it can be to get started with DX12, especially with the limited resources available. Microsoft’s samples and the Mini Engine are helpful, but they aren’t the best starting points if you’re new to DX12. However, here are some resources that can help you get started (trust me, that initial momentum is key):

- [3D GEP (By Jeremiah)](https://www.3dgep.com/category/graphics-programming/directx/)
- [DirectX Samples](https://github.com/microsoft/DirectX-Graphics-Samples)

My advice is to go through Jeremiah’s tutorials first. They’ll give you a solid foundation before you dive into the samples. The samples also include the "MiniEngine," which demonstrates a lot of DX12 features, but I’d recommend coming back to it once you’ve worked through the tutorials and feel comfortable with the basics.

Please do note that I am taking most of the code from [Cauldron](https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron) because it's neat and easy to understand. However, I am
not just copying and pasting from there, rather understanding the code first and modifying it to fit my needs.

## That’s about it for now
