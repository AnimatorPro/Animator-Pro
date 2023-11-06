#define GENERATE_CTYPE_TABLE

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../poco.h"
#include "aaconfig.h"
#include "pocoface.h"
#include "ptrmacro.h"
#include "xfile.h"

#ifdef _MSC_VER
#include <float.h>
#endif

#ifdef __APPLE__
/* Empty implementation for now-- seems to be a Windows-only thing? */
static void _fpreset() {}
#endif

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_metal.h"
#include <SDL.h>


// ======================================================================
static void ShowExampleMenuFile()
{
	ImGui::MenuItem("(demo menu)", NULL, false, false);
	if (ImGui::MenuItem("New")) {}
	if (ImGui::MenuItem("Open", "Ctrl+O")) {}
	if (ImGui::BeginMenu("Open Recent"))
	{
		ImGui::MenuItem("fish_hat.c");
		ImGui::MenuItem("fish_hat.inl");
		ImGui::MenuItem("fish_hat.h");
		if (ImGui::BeginMenu("More.."))
		{
			ImGui::MenuItem("Hello");
			ImGui::MenuItem("Sailor");
			if (ImGui::BeginMenu("Recurse.."))
			{
				ShowExampleMenuFile();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S")) {}
	if (ImGui::MenuItem("Save As..")) {}

	ImGui::Separator();
	if (ImGui::BeginMenu("Options"))
	{
		static bool enabled = true;
		ImGui::MenuItem("Enabled", "", &enabled);
		ImGui::BeginChild("child", ImVec2(0, 60), true);
		for (int i = 0; i < 10; i++) {
			ImGui::Text("Scrolling Text %d", i);
		}
		ImGui::EndChild();
		static float f = 0.5f;
		static int n = 0;
		ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
		ImGui::InputFloat("Input", &f, 0.1f);
		ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Colors"))
	{
		float sz = ImGui::GetTextLineHeight();
		for (int i = 0; i < ImGuiCol_COUNT; i++)
		{
			const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
			ImGui::Dummy(ImVec2(sz, sz));
			ImGui::SameLine();
			ImGui::MenuItem(name);
		}
		ImGui::EndMenu();
	}

	// Here we demonstrate appending again to the "Options" menu (which we already created above)
	// Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
	// In a real code-base using it would make senses to use this feature from very different code locations.
	if (ImGui::BeginMenu("Options")) // <-- Append!
	{
		static bool b = true;
		ImGui::Checkbox("SomeOption", &b);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Disabled", false)) // Disabled
	{
		IM_ASSERT(0);
	}
	if (ImGui::MenuItem("Checked", NULL, true)) {}
	ImGui::Separator();
	if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}


// ======================================================================
static void ShowExampleAppMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ShowExampleMenuFile();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}


// ======================================================================
int main(int argc, char** argv) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	// Setup SDL
	// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
	// depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

	// Enable native IME.
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL+Metal example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (window == nullptr)
	{
		printf("Error creating window: %s\n", SDL_GetError());
		return -2;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr)
	{
		printf("Error creating renderer: %s\n", SDL_GetError());
		return -3;
	}

	// Setup Platform/Renderer backends
	CAMetalLayer* layer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(renderer);
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	ImGui_ImplMetal_Init(layer.device);
	ImGui_ImplSDL2_InitForMetal(window);

	id<MTLCommandQueue> commandQueue = [layer.device newCommandQueue];
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

	// Our state
	float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

	// Main loop
	bool done = false;
	while (!done)
	{
		@autoreleasepool
		{
			// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				if (event.type == SDL_QUIT)
					done = true;
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
					done = true;
			}

			int width, height;
			SDL_GetRendererOutputSize(renderer, &width, &height);
			layer.drawableSize = CGSizeMake(width, height);
			id<CAMetalDrawable> drawable = [layer nextDrawable];

			id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
			renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
			renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
			renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
			renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
			id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
			[renderEncoder pushDebugGroup:@"ImGui demo"];

			// Start the Dear ImGui frame
			ImGui_ImplMetal_NewFrame(renderPassDescriptor);
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			// 1. Do file menu
			ShowExampleMenuFile();


			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
//				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
//				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::End();
			}


			// Rendering
			ImGui::Render();
			ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

			[renderEncoder popDebugGroup];
			[renderEncoder endEncoding];

			[commandBuffer presentDrawable:drawable];
			[commandBuffer commit];
		}
	}

	// Cleanup
	ImGui_ImplMetal_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}