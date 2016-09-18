import bindings from 'bindings';
const addon = bindings('addon');

const hello = () => {
  return addon.hello();
};

export default hello;
