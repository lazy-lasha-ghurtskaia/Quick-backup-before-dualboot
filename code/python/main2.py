import yt_dlp

url = "https://www.youtube.com/watch?v=30LWjhZzg50"

ydl_opts = {
    "outtmpl": "video.%(ext)s",  # output filename
    "format": "bestvideo+bestaudio/best",  # merge best video + audio
    "merge_output_format": "mp4"
}

with yt_dlp.YoutubeDL(ydl_opts) as ydl:
    ydl.download([url])

