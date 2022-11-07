#import <Foundation/Foundation.h>
#import <Foundation/NSString.h>

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

