// (C) Copyright 2014 by Autodesk, Inc.

//!\ReForm_SDK
#ifndef DEFINE_ERROR
#error This file should not be included directly, include the corresponding Outcome/Error header instead
#endif

// Make sure the first error code starts from 1, 0 is reserved for success, 
// and it is not an error code.
DEFINE_ERROR(SUCCESS, "Success")
DEFINE_ERROR(TODO, "TODO: Undefined error message")
DEFINE_ERROR(PROGRESS_ABORTED, "Progress aborted")
