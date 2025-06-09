import streamlit as st
import subprocess
import os
import time

st.set_page_config(page_title="Huffman Compressor", layout="wide")

st.title("🚀 Huffman Compressor & Decompressor")


st.markdown("Upload a file and select whether you want to **compress** or **decompress** it.")

mode = st.selectbox("Choose Mode", ["Compress", "Decompress"])

# COMPRESSION MODE
if mode == "Compress":
    uploaded_file = st.file_uploader("Upload a text file to compress", type=["txt"])

    if uploaded_file is not None:
        with open("input.txt", "wb") as f:
            f.write(uploaded_file.read())

        st.success("File uploaded successfully!")

        if st.button("🔒 Compress"):
            start = time.time()
            result = subprocess.run(["./huffman_exe", "compress", "input.txt", "compressed.huff"], capture_output=True, text=True)
            end = time.time()

            if result.returncode == 0:
                st.code(result.stdout, language="cpp")
                st.info(f"✅ Compression completed in {end - start:.2f} seconds")

                with open("compressed.huff", "rb") as f:
                    st.download_button("📥 Download Compressed File", f, file_name="compressed.huff")
            else:
                st.error("❌ Compression failed!")
                st.text(result.stderr)

# DECOMPRESSION MODE
elif mode == "Decompress":
    uploaded_compressed_file = st.file_uploader("Upload a .huff file to decompress", type=["huff"])

    if uploaded_compressed_file is not None:
        with open("compressed.huff", "wb") as f:
            f.write(uploaded_compressed_file.read())

        st.success("Compressed file uploaded successfully!")

        if st.button("🔓 Decompress"):
            start = time.time()
            result = subprocess.run(["./huffman_exe", "decompress", "compressed.huff", "decoded.txt"], capture_output=True, text=True)
            end = time.time()

            if result.returncode == 0:
                st.code(result.stdout, language="cpp")
                st.info(f"✅ Decompression completed in {end - start:.2f} seconds")

                with open("decoded.txt", "rb") as f:
                    st.download_button("📥 Download Decompressed File", f, file_name="decoded.txt")
            else:
                st.error("❌ Decompression failed!")
                st.text(result.stderr)

# Optional: Footer
st.markdown("---")
st.caption("Built  using Streamlit and C++")
