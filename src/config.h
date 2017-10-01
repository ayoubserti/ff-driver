#ifndef __FF_DRIVER_CONFIG__
#define __FF_DRIVER_CONFIG__




#ifndef EXPORT_API
#define PUBLIC_API __declspec(dllimport)
#define TEMPLATE_EXPORT_API extern
#else
#define PUBLIC_API __declspec(dllexport)
#define TEMPLATE_EXPORT_API extern
#endif

#endif