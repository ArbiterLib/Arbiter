{-# LANGUAGE ForeignFunctionInterface #-}

module Arbiter.Version where

import Foreign
import Foreign.C

#include "arbiter/Version.h"

data ArbiterSemanticVersion = ArbiterSemanticVersion
  { _major'ArbiterSemanticVersion :: Int
  , _minor'ArbiterSemanticVersion :: Int
  , _patch'ArbiterSemanticVersion :: Int
  }

instance Storable ArbiterSemanticVersion where
  sizeOf _ = {#sizeof ArbiterSemanticVersion #}
  alignment _ = {#alignof ArbiterSemanticVersion #}
  peek p = ArbiterSemanticVersion
    <$> liftM fromIntegral ({#get ArbiterSemanticVersion->_major #} p)
    <*> liftM fromIntegral ({#get ArbiterSemanticVersion->_minor #} p)
    <*> liftM fromIntegral ({#get ArbiterSemanticVersion->_patch #} p)
  poke p x = do
    {#set ArbiterSemanticVersion._major #} p (fromIntegral $ _major'ArbiterSemanticVersion x)
    {#set ArbiterSemanticVersion._minor #} p (fromIntegral $ _minor'ArbiterSemanticVersion x)
    {#set ArbiterSemanticVersion._patch #} p (fromIntegral $ _patch'ArbiterSemanticVersion x)
