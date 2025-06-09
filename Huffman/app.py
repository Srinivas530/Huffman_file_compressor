import streamlit as st
import subprocess
import os
import time

st.set_page_config(page_title="Huffman Compressor", layout="wide")
st.title("🚀 Huffman Compressor & Decompressor")
st.markdown("Upload a file and select whether you want to **compress** or **decompress** it.")

# Step 1: Compile the C++ binary (only once if not compiled)
binary_path = os.path.join("Huffman", "huffman_exe")
source_path = os.path.join("Huffman", "huff8.cpp")

if not os.path.exists(binary_path):
    st.info("🔧 Compiling C++ Huffman binary...")
    compile_result = subprocess.run(["g++", "-O2", "-o", binary_path, source_path], capture_output=True, text=True)
    if compile_result.returncode != 0:
        st.error("❌ Compilation failed!")
        st.text(compile_result.stderr)
        st.stop()
    else:
        st.success("✅ Compilation successful!")

# Step 2: Interface
mode = st.selectbox("Choose Mode", ["Compress", "Decompress"])

if mode == "Compress":
    uploaded_file = st.file_uploader("Upload a text file to compress", type=["txt"])
    if uploaded_file is not None:
        with open("input.txt", "wb") as f:
            f.write(uploaded_file.read())
        st.success("File uploaded successfully!")

        if st.button("🔒 Compress"):
            start = time.time()
            result = subprocess.run([binary_path, "compress", "input.txt", "compressed.huff"], capture_output=True, text=True)
            end = time.time()

            if result.returncode == 0:
                st.code(result.stdout)
                st.info(f"✅ Compression completed in {end - start:.2f} seconds")
                with open("compressed.huff", "rb") as f:
                    st.download_button("📥 Download Compressed File", f, file_name="compressed.huff")
            else:
                st.error("❌ Compression failed!")
                st.text(result.stderr)

elif mode == "Decompress":
    uploaded_compressed_file = st.file_uploader("Upload a .huff file to decompress", type=["huff"])
    if uploaded_compressed_file is not None:
        with open("compressed.huff", "wb") as f:
            f.write(uploaded_compressed_file.read())
        st.success("Compressed file uploaded successfully!")

        if st.button("🔓 Decompress"):
            start = time.time()
            result = subprocess.run([binary_path, "decompress", "compressed.huff", "decoded.txt"], capture_output=True, text=True)
            end = time.time()

            if result.returncode == 0:
                st.code(result.stdout)
                st.info(f"✅ Decompression completed in {end - start:.2f} seconds")
                with open("decoded.txt", "rb") as f:
                    st.download_button("📥 Download Decompressed File", f, file_name="decoded.txt")
            else:
                st.error("❌ Decompression failed!")
                st.text(result.stderr)

# Footer
st.markdown("---")
st.caption("Built using Streamlit and C++")
