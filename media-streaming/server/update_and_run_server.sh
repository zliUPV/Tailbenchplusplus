#FROM ubuntu:18.04
#Original maintainer: MAINTAINER Tapti Palit <tpalit@cs.stonybrook.edu>
#Current maintainer: Puche 


apt-get update -y && apt-get install -y --no-install-recommends \
      nginx \
    && rm -rf /var/lib/apt/lists/*

# Increase the open file limit
cp files/limits.conf.append /tmp/
cat /tmp/limits.conf.append >> /etc/security/limits.conf

# Update nginx to serve /videos
sed -i 's|/usr/share/nginx/html|/videos|g' /etc/nginx/sites-available/default

nginx -g "daemon off; pid logs/nginx.pid;"

# CMD ["nginx", "-g", "daemon off;"]
~                                     
