#!/bin/bash
echo "Input name of new module"
read -r NEW_MODULE_NAME
NEW_MODULE_NAME_UPPER=${NEW_MODULE_NAME^^}
echo $NEW_MODULE_NAME_UPPER
BACKEND_DIR=./module_name/backend
mv "$BACKEND_DIR/module_name.cpp" "$BACKEND_DIR/$NEW_MODULE_NAME.cpp"
mv "$BACKEND_DIR/module_name.h" "$BACKEND_DIR/$NEW_MODULE_NAME.h"
mv "$BACKEND_DIR/module_name_wrapper.cpp" "$BACKEND_DIR/$NEW_MODULE_NAME""_wrapper.cpp"
mv ./module_name $NEW_MODULE_NAME

replace_module_name()
{
    FILENAME=$1
    NEW=$2
    NEW_UPPER=$3
    echo $filename
    sed -i s/module_name/$NEW/g $FILENAME
    sed -i s/MODULE_NAME/$NEW_UPPER/g $FILENAME
}
#rename files
for filename in $NEW_MODULE_NAME/backend/*
do
  replace_module_name $filename $NEW_MODULE_NAME $NEW_MODULE_NAME_UPPER
done

replace_module_name CMakeLists.txt $NEW_MODULE_NAME $NEW_MODULE_NAME_UPPER
replace_module_name pyproject.toml $NEW_MODULE_NAME $NEW_MODULE_NAME_UPPER
replace_module_name setup.py $NEW_MODULE_NAME $NEW_MODULE_NAME_UPPER

#delete old git repository
rm -rf .git
git init .
git add .
git commit -m "initial commit"

#create venv
PYTHON_INTERPRETER=python3.10
echo "Setup Python Virtual Environment"

if [ -d ".venv" ]; then
    rm -rf .venv
fi

echo "Installing Submodules"

# create new virtual environment
${PYTHON_INTERPRETER} -m venv .venv && \
source .venv/bin/activate
pip install pre-commit
pre-commit install

#clean README.md
rm README.md
touch README.md
#self delete this script
rm fill_template.sh
