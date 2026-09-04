SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

function AddToLibraryPath()
{
    export LD_LIBRARY_PATH="${SCRIPT_DIR}/${1}:${LD_LIBRARY_PATH}"
    export DYLD_LIBRARY_PATH="${SCRIPT_DIR}/${1}:${DYLD_LIBRARY_PATH}"
}

AddToLibraryPath src/