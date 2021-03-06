#include "common/runtime.h"

extern "C"
{
    void userAppInit()
    {
        Result res = plInitialize(PlServiceType_User);

        if (R_FAILED(res))
            fatalThrow(res);

        res = nifmInitialize(NifmServiceType_User);

        if (R_FAILED(res))
            fatalThrow(res);

        #if !defined(EMULATION)
            res = accountInitialize(AccountServiceType_Application);

            if (R_FAILED(res))
                fatalThrow(res);
        #endif

        res = setInitialize();

        if (R_FAILED(res))
            fatalThrow(res);

        #if !defined(EMULATION)
            res = psmInitialize();

            if (R_FAILED(res))
                fatalThrow(res);
        #endif

        res = socketInitializeDefault();

        if (R_FAILED(res))
            fatalThrow(res);

        TTF_Init();
    }

    void userAppExit()
    {
        socketExit();
        psmExit();
        setExit();
        accountExit();
        nifmExit();
        plExit();

        // Deinitialize everything else
        TTF_Quit();
    }
}
