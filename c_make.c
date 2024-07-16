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
            c_make_command_append_command_line(&command, c_make_get_target_c_flags());

            if (c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, "-nologo");
            }
            else
            {
                c_make_command_append(&command, "-std=c99");
            }

            CMakeBuildType build_type = c_make_get_build_type();

            switch (build_type)
            {
                case CMakeBuildTypeDebug:
                {
                    if (c_make_compiler_is_msvc(target_c_compiler))
                    {
                        c_make_command_append(&command, "-Od");
                    }
                    else
                    {
                        c_make_command_append(&command, "-Wall", "-g", "-O0");
                    }
                } break;

                case CMakeBuildTypeRelDebug:
                {
                    if (c_make_compiler_is_msvc(target_c_compiler))
                    {
                        c_make_command_append(&command, "-O2");
                    }
                    else
                    {
                        c_make_command_append(&command, "-g", "-O2");
                    }
                } break;

                case CMakeBuildTypeRelease:
                {
                    c_make_command_append(&command, "-O2", "-DNDEBUG");
                } break;
            }

            if (c_make_compiler_is_msvc(target_c_compiler))
            {
                c_make_command_append(&command, c_make_c_string_concat("-Fe", c_make_c_string_path_concat(c_make_get_build_path(), "juls.exe")));
                c_make_command_append(&command, c_make_c_string_concat("-Fo", c_make_c_string_path_concat(c_make_get_build_path(), "juls.obj")));
                c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "src", "main.c"));
            }
            else
            {
                c_make_command_append(&command, "-o", c_make_c_string_path_concat(c_make_get_build_path(), "juls"));
                c_make_command_append(&command, c_make_c_string_path_concat(c_make_get_source_path(), "src", "main.c"));
            }

            c_make_log(CMakeLogLevelInfo, "compile 'juls'\n");
            c_make_command_run_and_wait(command);
        } break;

        case CMakeTargetInstall:
        {
        } break;
    }
}
