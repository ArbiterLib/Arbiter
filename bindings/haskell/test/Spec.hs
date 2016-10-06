import Arbiter.Version
  ( createSemanticVersion
  , getMajorVersion
  , getMinorVersion
  , getPatchVersion
  )

main :: IO ()
main = do
  let major = 1
  let minor = 2
  let patch = 3
  let prereleaseVersion = "alpha.0"
  let buildMetadata = "dailybuild"
  let version = createSemanticVersion major minor patch prereleaseVersion buildMetadata
  putStrLn $ show $ getMajorVersion version
  putStrLn $ show $ getMinorVersion version
  putStrLn $ show $ getPatchVersion version
