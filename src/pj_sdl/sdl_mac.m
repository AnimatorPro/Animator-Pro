#import <Foundation/Foundation.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>
#include <SDL3/SDL_filesystem.h>


// ======================================================================
const char* pj_sdl_mac_bundle_path() {
	static const char resource_path[PATH_MAX] = {0};

	#ifdef IS_BUNDLE
	static NSString* resource_path = nil;
	if (!resource_path) {
		resource_path = [[NSBundle mainBundle] resourcePath];
	}
	snprintf(resource_path, PATH_MAX, [resource_path UTF8String]);
	#endif

	return resource_path;
}


// ----------------------------------------------------------------------
const char* mac_preferences_path() {
	static NSString* preferences_path = nil;

	fprintf(stderr, "SDL PATHS:\n\\t+ Base: %s\n\t+ Prefs: %s\n\n",
			SDL_GetBasePath(),
			SDL_GetPrefPath("skeletonheavy", "vpaint"));

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