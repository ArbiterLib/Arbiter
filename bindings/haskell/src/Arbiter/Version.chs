{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version
  ( createSemanticVersion
  , getMajorVersion
  , getMinorVersion
  , getPatchVersion
  ) where

#include "arbiter/Types.h"
#include "arbiter/Version.h"

{#pointer *ArbiterSemanticVersion as SemanticVersionPtr
  foreign finalizer ArbiterFree as arbiterFree newtype#}

-- | Creates a semantic version with the given components.
{#fun pure ArbiterCreateSemanticVersion as createSemanticVersion
  { `Int', `Int', `Int', `String', `String' } -> `SemanticVersionPtr' #}

-- | Returns the major version number (X.y.z) from a semantic version.
{#fun pure ArbiterGetMajorVersion as getMajorVersion
  { `SemanticVersionPtr' } -> `Int' #}

-- | Returns the minor version number (x.Y.z) from a semantic version.
{#fun pure ArbiterGetMinorVersion as getMinorVersion
  { `SemanticVersionPtr' } -> `Int' #}

-- | Returns the patch version number (x.y.Z) from a semantic version.
{#fun pure ArbiterGetPatchVersion as getPatchVersion
  { `SemanticVersionPtr' } -> `Int' #}
