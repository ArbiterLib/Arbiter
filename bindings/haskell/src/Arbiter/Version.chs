{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

#include "arbiter/Version.h"

type SemanticVersion = {#type ArbiterSemanticVersion#}

{#fun pure ArbiterCreateSemanticVersion as ^
  { `Int', `Int', `Int', `String', `String' } -> `()' #}
