{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

#include "arbiter/Version.h"

{#pointer *ArbiterSemanticVersion as SemanticVersion newtype#}

{#fun pure ArbiterCreateSemanticVersion as ^
  { `Int', `Int', `Int', `String', `String' } -> `SemanticVersion' #}
