#import <Foundation/Foundation.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>


// ======================================================================
const char* mac_resources_path() {
#ifdef IS_BUNDLE
	static NSString* resource_path = nil;
	if (!resource_path) {
		resource_path = [[NSBundle mainBundle] resourcePath];
	}

	return [resource_path UTF8String];
#else
	static const char* resource_path = "";
	return resource_path;
#endif
}


// ----------------------------------------------------------------------
const char* mac_preferences_path() {
	static NSString* preferences_path = nil;

	if (!preferences_path) {
		NSFileManager* file_manager = [NSFileManager defaultManager];
		NSURL* result_url			= [file_manager URLForDirectory:NSApplicationSupportDirectory
												   inDomain:NSUserDomainMask
										  appropriateForURL:nil
													 create:NO
													  error:nil];

		result_url = [result_url URLByAppendingPathComponent:@"com.skeletonheavy.animator-pro"];

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