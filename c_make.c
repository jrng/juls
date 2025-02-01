#define C_MAKE_IMPLEMENTATION
#include "c_make.h"

C_MAKE_ENTRY()
{
    switch (c_make_target)
    {
        case CMakeTargetSetup:
        {
        } break;

        case CMakeTargetBuild:
        {
            CMakeCommand command = { 0 };

            const char *target_c_compiler = c_make_get_target_c_compiler();

            c_make_command_append(&command, target_c_compiler);

            CMakeBuildType build_type = c_make_get_build_type();

            if (!c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-std=c99");

                if (build_type == CMakeBuildTypeDebug)
                {
                    c_make_command_append(&command, "-Wall");
                }
            }

            c_make_command_append_command_line(&command, c_make_get_target_c_flags());
            c_make_command_append_default_compiler_flags(&command, build_type);
            c_make_command_append_output_executable(&command, c_make_c_string_path_concat(c_make_get_build_path(), "juls"), c_make_get_target_platform());
            c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "src", "main.c"));

            c_make_log(CMakeLogLevelInfo, "compile 'juls'\n");
            c_make_command_run(command);
        } break;

        case CMakeTargetInstall:
        {
        } break;
    }
}
