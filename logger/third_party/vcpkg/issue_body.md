Package: fmt:x64-windows@11.0.2#1

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35215.0
-    vcpkg-tool version: 2025-07-21-d4b65a2b83ae6c3526acd1c6f3b51aff2a884533
    vcpkg-scripts version: ae1529e98a 2025-08-29 (4 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached fmtlib-fmt-11.0.2.tar.gz

```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "my-project",
  "version-string": "1.0.0",
  "dependencies": [
    {
      "name": "fmt",
      "version>=": "11.0.2"
    }
  ]
}

```
</details>
