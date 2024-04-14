---
css:
  - node_modules/github-fork-ribbon-css/gh-fork-ribbon.css
  - css/custom.css
  - css/animations.css
  - slides/modern-js-addons/modern-js-addons.css
scripts:
  - node_modules/jquery/dist/jquery.min.js
  - scripts/customize.js
  - node_modules/reveal-compiler-explorer/dist/reveal-compiler-explorer.js
  - scripts/load-plugins.js
  - scripts/custom-options.js
---

<!-- .slide: data-background-image="assets/accu_title.png" -->

---

<!-- .slide: data-background-image="assets/parquet-viewer.png" data-background-size="contain" -->

---

<!-- .slide: data-background-image="assets/calendar.png" data-background-size="contain" -->

---


<!-- .slide: data-background-image="assets/accu_2023.png" data-background-size="contain" -->

----

## Node.js Addons

- shared libraries loaded by node.js
- can be imported like node.js modules
- [Node-API](https://nodejs.org/api/n-api.html) C API
- [node-addon-api](https://github.com/nodejs/node-addon-api) C++ header only API

Notes:

Addons are dynamically-linked shared objects that can be loaded into Node.js like JavaScript modules. 

Node-API (formerly N-API) is an API for building native Addons. Unlike older APIs (like nan), it is independent from the underlying JavaScript runtime (for example, V8) and is maintained as part of Node.js itself. This API will be Application Binary Interface (ABI) stable across versions of Node.js. 

There's also other, unofficial, bindings for other languages like rust, swift and zig.

---

## building a C++ addon

- `node-gyp`
- `CMake.js`

Notes:

node-gyp is the official build system from the Node.js org. It is based on the chromium build system, written in Python and is wide and has widespread adoption and documentation. 

However, for C++ developers, it might be more convenient to use CMake, which is a more modern and powerful build system that is widely used in the C++ community.
For that, we can use CMake.js, which is a CMake wrapper integrating with the Node.js ecosystem.

Both tools require a C++ toolchain to be installed on the system. For node-gyp, Python is required and for CMake.js, CMake is required.

---

![`Pipenv`](assets/pipenv.png)

<!-- .element: class="r-stretch" -->

```bash
pipenv install cmake
pipenv run cmake
```

Notes:

`Pipenv` is a tool that automatically creates and manages virtual Python environments. It's main advantages here is the ability to run installed python packages simply with the `pipenv run`command.

---

`Pipfile`

```ini[|7]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/Pipfile
```

<!--.element: class="r-stretch" -->

---

`package.json`

```json[16-17,19-21|11-12|5-9]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/package.json
```

<!--.element: class="r-stretch" -->

---

## Node-Api version [matrix](https://nodejs.org/api/n-api.html#node-api-version-matrix)

<table style="font-size: 0.8em" -->
  <tr>
    <th>Node-API version</th>
    <th scope="col">Supported In</th>
  </tr>
  <tr>
    <th scope="row">9</th>
    <td>v18.17.0+, 20.3.0+, 21.0.0 and all later versions</td>
  </tr>
  <tr>
    <th scope="row">8</th>
    <td>v12.22.0+, v14.17.0+, v15.12.0+, 16.0.0 and all later versions</td>
  </tr>
  <tr>
    <th scope="row">7</th>
    <td>v10.23.0+, v12.19.0+, v14.12.0+, 15.0.0 and all later versions</td>
  </tr>
  <tr>
    <th scope="row">6</th>
    <td>v10.20.0+, v12.17.0+, 14.0.0 and all later versions</td>
  </tr>
</table>

---

`.npmrc`

```ini[12-14]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/.npmrc
```

<!--.element: class="r-stretch" -->

---

`CMakeLists.txt`

```cmake[1-3|5-24|39-47]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/cpp/CMakeLists.txt
```

<!--.element: class="r-stretch" -->

---

`inteface.cpp`

```cpp[1|37-44|9-23|25-34|7]
///skip
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/cpp/interface.cpp
```

<!--.element: class="r-stretch" -->

Notes: 

It's advised to return after calling `ThrowAsJavaScriptException` in case user compiles with exceptions disabled.

---

`calendar.cxx`

```cpp[1-11|81-83|166-179|113-124]
///skip
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/cpp/calendar.cxx
```

<!--.element: class="r-stretch" -->

---

`concat.cxx`

```cpp[1-7|37-41]
///skip
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/cpp/concat.cxx
```

<!--.element: class="r-stretch" -->

---

![conan](assets/conan2-logo-for-dark.svg)

```bash
pipenv install conan
```

Notes: 

Conan is a C++ package manager that allows developers to easily manage dependencies and build artifacts for C++ projects. It is widely used in the C++ community and integrates with many build systems and package managers.

---

`.npmrc`

```ini[15]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/.npmrc
```

<!--.element: class="r-stretch" -->

<!-- .element: style="font-size: 0.45em" -->

https://github.com/conan-io/cmake-conan

---

`CMakeLists.txt`

```cmake[29-37,50]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/cpp/CMakeLists.txt
```

<!--.element: class="r-stretch" -->

---

## demo

----

## deployment

- `node-pre-gyp`
- `prebuild`
- `prebuildify`
- `pkg-prebuilds`

Notes:

`node-pre-gyp` is a tool based on node-gyp that adds the ability to upload binaries to a server of the developer's choice. node-pre-gyp has particularly good support for uploading binaries to Amazon S3.

`prebuild` is a tool that supports builds using either node-gyp or CMake.js. Unlike node-pre-gyp which supports a variety of servers, `prebuild` uploads binaries only to GitHub releases. `prebuild` is a good choice for GitHub projects using CMake.js.

`prebuildify` is a tool based on node-gyp. The advantage of `prebuildify` is that the built binaries are bundled with the native addon when it's uploaded to npm. The binaries are downloaded from npm and are immediately available to the module user when the native addon is installed.

`pkg-prebuild` is built to support cmake-js and be simple.

---

`binding-options.ts`

```ts[]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/ts/binding-options.ts
```

<!--.element: class="r-stretch" -->

---

`package.json`

```json[12]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/package.json
```

<!--.element: class="r-stretch" -->

---

`CMakeLists.txt`

```cmake[52-55]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/cpp/CMakeLists.txt
```

<!--.element: class="r-stretch" -->

---

`interface.ts`

```ts[]
FILE: modern-js-addons/electron-calendar/packages/calendar-generator/src/ts/interface.ts
```

<!--.element: class="r-stretch" -->

---

## GitHub actions

![pipeline](assets/pipeline.png)

---

## GitHub actions

```yml [1-11|14-23]
build:
    runs-on: ${{ matrix.os }}
    steps:
      ...
      - name: Upload prebuilds
        if: '!env.ACT'
        uses: actions/upload-artifact@v3
        with:
          name: prebuilds
          path: packages/parquet-reader/prebuilds
          retention-days: 1


release:
    runs-on: ubuntu-latest
    needs:
      - build
    steps:
      ...
      - uses: actions/download-artifact@v3
        with:
          name: prebuilds
          path: packages/parquet-reader/prebuilds

```

----

<!-- .slide: data-background-image="assets/accu_thank_you.png" -->

