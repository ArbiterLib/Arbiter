{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/hello.cc" ]
    },
    {
      "target_name": "arbiter-node-bindings",
      "include_dirs": [
        "../../include"
      ],
      "libraries": [
        "../../../libArbiter.a"
      ]
    }
  ]
}
