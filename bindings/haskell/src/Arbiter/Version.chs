{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

#include "arbiter/Types.h"
#include "arbiter/Version.h"

{#pointer *ArbiterSemanticVersion as SemanticVersionPtr
  foreign finalizer ArbiterFree as arbiterFree newtype#}

{#fun pure ArbiterCreateSemanticVersion as ^
  { `Int', `Int', `Int', `String', `String' } -> `SemanticVersionPtr' #}
