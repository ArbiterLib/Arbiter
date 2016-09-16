import Distribution.Simple
    ( Args
    , defaultMainWithHooks
    , confHook
    , preBuild
    , simpleUserHooks
    )
import Data.Maybe (fromJust)
import qualified Distribution.PackageDescription as PD
import Distribution.Simple.LocalBuildInfo
    ( LocalBuildInfo
    , localPkgDescr
    )
import Distribution.Simple.Setup
    ( BuildFlags
    , ConfigFlags
    , buildVerbosity
    , fromFlag
    )
import Distribution.Simple.Utils (rawSystemExit)
import System.Directory (getCurrentDirectory)
import System.FilePath (takeDirectory)

main = defaultMainWithHooks simpleUserHooks
    { confHook = arbiterConfHook
    , preBuild = \a b -> makeLib a b >> preBuild simpleUserHooks a b
    }

makeLib :: Args -> BuildFlags -> IO ()
makeLib _ flags =
    rawSystemExit (fromFlag $ buildVerbosity flags) "env"
        ["make", "build", "--directory=../.."]


arbiterConfHook :: (PD.GenericPackageDescription, PD.HookedBuildInfo) ->
                   ConfigFlags ->
                   IO LocalBuildInfo
arbiterConfHook (description, buildInfo) flags = do
    localBuildInfo <- confHook simpleUserHooks (description, buildInfo) flags
    let packageDescription = localPkgDescr localBuildInfo
        library = fromJust $ PD.library packageDescription
        libraryBuildInfo = PD.libBuildInfo library
    currentDir <- getCurrentDirectory
    let dir = (takeDirectory . takeDirectory) currentDir
    return localBuildInfo
        { localPkgDescr = packageDescription
            { PD.library = Just $ library
                { PD.libBuildInfo = libraryBuildInfo
                    { PD.includeDirs = (dir ++ "/include"):PD.includeDirs libraryBuildInfo
                    , PD.extraLibDirs = dir:PD.extraLibDirs libraryBuildInfo
                    }
                }
            }
        }
