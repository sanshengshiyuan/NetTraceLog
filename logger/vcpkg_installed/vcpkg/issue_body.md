Package: openssl:x64-windows@3.5.1

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35215.0
-    vcpkg-tool version: 2025-07-21-d4b65a2b83ae6c3526acd1c6f3b51aff2a884533
    vcpkg-scripts version: 120deac306 2025-08-27 (2 days ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached openssl-openssl-openssl-3.5.1.tar.gz

```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "mylog-logger",
  "version": "1.0.0",
  "dependencies": [
    "openssl"
  ]
}

```
</details>
