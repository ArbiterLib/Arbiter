import Distribution.Simple
    ( Args
    , defaultMainWithHooks
    , preBuild
    , simpleUserHooks
    )
import Distribution.Simple.Setup
    ( BuildFlags
    , buildVerbosity
    , fromFlag
    )
import Distribution.Simple.Utils (rawSystemExit)

main = defaultMainWithHooks simpleUserHooks
    { preBuild = \a b -> makeLib a b >> preBuild simpleUserHooks a b
    }

makeLib :: Args -> BuildFlags -> IO ()
makeLib _ flags =
    rawSystemExit (fromFlag $ buildVerbosity flags) "env"
        ["make", "build", "--directory=../.."]
