gource --multi-sampling -1920x1080 --seconds-per-day 1 --stop-at-end --user-image-dir avatars -o gource.ppm 
ffmpeg -y -r 60 -f image2pipe -vcodec ppm -pix_fmt rgb24 -s 1920x1080 -i gource.ppm -vcodec libx264 -preset ultrafast -pix_fmt yuv420p -crf 1 -threads 0 -bf 0 gource.mp4
rm gource.ppm