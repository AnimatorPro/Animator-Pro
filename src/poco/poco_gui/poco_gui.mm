#define GENERATE_CTYPE_TABLE

#import <Foundation/Foundation.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_memory_editor.h"
#include <SDL3/SDL.h>

#include "../poco.h"
#include "../../pj_sdl/pj_sdl.h"
#include <TextEditor.h>

#include <nfd.h>

#ifdef _MSC_VER
#include <float.h>
#endif

#ifdef __APPLE__
/* Empty implementation for now-- seems to be a Windows-only thing? */
static void _fpreset() {}
#endif

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
typedef struct _dockIds {
	ImGuiID root = 0;
	ImGuiID memedit = 0;
	ImGuiID text = 0;
	ImGuiID console = 0;
} DockIDs;

DockIDs dockIds;

ImGuiID setup_docking() {
	if (ImGui::DockBuilderGetNode(dockIds.root) == 0) {
		dockIds.root = ImGui::GetID("Root_Dockspace");

		ImGui::DockBuilderRemoveNode(dockIds.root); // Clear out existing layout
		ImGui::DockBuilderAddNode(dockIds.root,
								  ImGuiDockNodeFlags_DockSpace); // Add empty node
		dockIds.memedit =
		  ImGui::DockBuilderSplitNode(dockIds.root, ImGuiDir_Right, 0.5f, NULL, &dockIds.root);
		dockIds.text =
		  ImGui::DockBuilderSplitNode(dockIds.root, ImGuiDir_Left, 0.2f, NULL, &dockIds.root);
		dockIds.console =
		  ImGui::DockBuilderSplitNode(dockIds.root, ImGuiDir_Down, 0.3f, NULL, &dockIds.root);

		ImGui::DockBuilderDockWindow("Poco", dockIds.root);
		ImGui::DockBuilderDockWindow("Text Editor", dockIds.root);
		ImGui::DockBuilderDockWindow("Stack", dockIds.memedit);
		ImGui::DockBuilderDockWindow("Console", dockIds.console);

		ImGui::DockBuilderFinish(dockIds.root);
	}

	return dockIds.root;
}


// ======================================================================
const char* mac_preferences_path() {
	static NSString* preferences_path = nil;

	if (!preferences_path) {
		NSFileManager* file_manager = [NSFileManager defaultManager];
		NSURL* result_url			= [file_manager URLForDirectory:NSApplicationSupportDirectory
												   inDomain:NSUserDomainMask
												   appropriateForURL:nil
													create:NO
													error:nil];

		result_url = [result_url URLByAppendingPathComponent:@"com.vpaint.animator-pro"];

		if (![file_manager createDirectoryAtURL:result_url withIntermediateDirectories:YES attributes:nil error:nil])
		{
			NSLog(@"Unable to create directory: %s.", [[result_url path] UTF8String]);
		}
		else {
			preferences_path = [result_url path];
		}
	}

	if (preferences_path) {
		return [preferences_path UTF8String];
	}

	return ".";
}


// ======================================================================
void open_file() {
	nfdchar_t *outPath;
	nfdfilteritem_t filterItem[2] = { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
	nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, "/Users/kiki/dev");
	if (result == NFD_OKAY)
	{
		puts("Success!");
		puts(outPath);
		NFD_FreePath(outPath);
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}

// ======================================================================
int main(int argc, char** argv) {
	fprintf(stderr, "%s\n\n", argv[0]);

	// global variables
	char active_file[1024];
	sprintf(active_file, "untitled.poc");
	char* active_file_contents = NULL;

	NFD_Init();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	// disable ini file and logging
//	io.IniFilename = NULL;
//	io.LogFilename = NULL;
	char ini_filename[1024];
	sprintf(ini_filename, "%s/%s", mac_preferences_path(), "imgui.ini");
	io.IniFilename = ini_filename;
	fprintf(stderr, "Imgui INI: %s\n", ini_filename);
//	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

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

	char font_name[256];
	getcwd(font_name, 256);
	sprintf(font_name, "%s/resource/hack.ttf", font_name);
	ImFont* hack = io.Fonts->AddFontFromFileTTF("resource/hack.ttf", 20);

	// Setup SDL
	// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
	// depending on whether SDL_INIT_GAMEPAD is enabled or disabled.. updating to latest version of SDL is recommended!)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

	// Enable native IME.
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	SDL_Window* window = SDL_CreateWindow("Poco GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
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
	CAMetalLayer* layer = (__bridge CAMetalLayer*)SDL_GetRenderMetalLayer(renderer);
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	ImGui_ImplMetal_Init(layer.device);
	ImGui_ImplSDL2_InitForMetal(window);

	id<MTLCommandQueue> commandQueue = [layer.device newCommandQueue];
	MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

	// Our state
	float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

	// Main loop

	bool done = false;
    bool show_about_window = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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
				if (event.type == SDL_EVENT_QUIT)
					done = true;
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_EVENT_WINDOW_CLOSE_REQUESTED
                        && event.window.windowID == SDL_GetWindowID(window))
					done = true;
			}

			int width, height;
			SDL_GetCurrentRenderOutputSize(renderer, &width, &height);
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

			ImGui::PushFont(hack);

			// 0. Main Window
			//    This is _required_ for the dockspace!
			ImGuiWindowFlags window_flags =
			  ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

			ImGuiViewport *viewport = ImGui::GetMainViewport();

			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
							ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |=
			  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::DockSpaceOverViewport(viewport);

//			ImGui::Begin("Poco", (bool *)0, window_flags);

			ImGui::PopStyleVar();
			ImGui::PopStyleVar(2);


			// 0. Do file menu
			if (ImGui::BeginMainMenuBar()) {

                if (ImGui::BeginMenu("File")) {
                    ImGui::MenuItem("About...", NULL, &show_about_window);
					if (ImGui::MenuItem("Open File...", "CMD+O", false, true)) {
						open_file();
					}
                    if (ImGui::MenuItem("Quit", "CMD+Q", false, true)) {
                        done = true;
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit")) {
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

			// 1. Do file menu
			if (show_about_window) {
				ImGui::ShowAboutWindow(&show_about_window);
			}

			// 2. Docking stuff
//			ImGuiID root_id = setup_docking();
//			ImGui::DockSpace(root_id, ImVec2(0.0f, 0.0f), dockspace_flags);


			// 0. Stack view
//			ImGuiWindowClass window_class;
//			window_class.DockNodeFlagsOverrideSet = 1 << 12; // no tab bar
//			ImGui::SetNextWindowClass(&window_class);

			ImGui::SetNextWindowDockID(dockIds.memedit, ImGuiCond_Appearing);

			static MemoryEditor memory_editor;
			static char data[0x10000];
			size_t data_size = 0x10000;
			memory_editor.DrawWindow("Stack", data, data_size);

			ImGui::SetNextWindowDockID(dockIds.root, ImGuiCond_Appearing);
			ImGui::Begin(active_file, (bool *)0);
			ImGui::Text("Text goes here");
			ImGui::End();

			ImGui::SetNextWindowDockID(dockIds.console, ImGuiCond_Appearing);
			ImGui::Begin("Console", (bool *)0);
			ImGui::Text("Console log goes here.");
			ImGui::End();


//			ImGui::End();
			ImGui::PopFont();

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

	NFD_Quit();

	return 0;
}