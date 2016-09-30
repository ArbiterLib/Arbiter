{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

#include "arbiter/Types.h"
#include "arbiter/Version.h"

  foreign finalizer ArbiterFree newtype#}
{#pointer *ArbiterSemanticVersion as SemanticVersionPtr

{#fun pure ArbiterCreateSemanticVersion as ^
  { `Int', `Int', `Int', `String', `String' } -> `SemanticVersionPtr' #}
