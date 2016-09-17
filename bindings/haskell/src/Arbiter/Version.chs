{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

#include "arbiter/Types.h"
#include "arbiter/Version.h"

{#pointer *ArbiterSemanticVersion as SemanticVersion
  foreign finalizer ArbiterFree newtype#}

{#fun pure ArbiterCreateSemanticVersion as ^
  { `Int', `Int', `Int', `String', `String' } -> `SemanticVersion' #}
