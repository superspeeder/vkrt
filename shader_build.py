import os
import subprocess
import io


if os.name == "nt":
    GLSLCNAME = "glslc.exe"
else:
    GLSLCNAME = "glslc"

def build_shader(src_path: str, glslc_path: str = GLSLCNAME):
    binpath = os.path.join("shaders", "bin", os.path.relpath(src_path, "shaders/src"))
    binpath += '.spv'

    asmpath = os.path.join("shaders", "spirvasm", os.path.relpath(src_path, "shaders/src"))
    asmpath += '.spv.s'

    p = subprocess.run([glslc_path, "--target-env=vulkan1.3", "-O", "-o", binpath, '-I', 'shaders/include/', src_path], capture_output=True)
    if p.returncode != 0:
        print(f"Failed to build {src_path}:\n{p.stderr.decode('utf-8')}")
        return False

    p = subprocess.run([glslc_path, "--target-env=vulkan1.3", "-O", "-o", asmpath, '-I', 'shaders/include/', '-S', src_path], capture_output=True)

    return p.returncode == 0


if __name__ == '__main__':
    import glob
    import multiprocessing

    if not os.path.exists("shaders/src"):
        raise Exception("No shaders found")

    os.makedirs("shaders/bin", exist_ok=True)
    os.makedirs("shaders/spirvasm", exist_ok=True)

    vksdk = os.getenv("VULKAN_SDK")
    if vksdk is not None:
        glslc = os.path.join(vksdk, "Bin", GLSLCNAME)
    else:
        glslc = GLSLCNAME

    files = glob.glob("shaders/src/**.*", recursive=True)

    for f in files:
        build_shader(f, glslc)
