{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

#include "arbiter/Types.h"
#include "arbiter/Version.h"

{#pointer *ArbiterSemanticVersion as SemanticVersionPtr
  foreign finalizer ArbiterFree as arbiterFree newtype#}

-- | Creates a semantic version with the given components.
{#fun pure ArbiterCreateSemanticVersion as createSemanticVersion
  { `Int', `Int', `Int', `String', `String' } -> `SemanticVersionPtr' #}
