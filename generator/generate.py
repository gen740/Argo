import re


base = "./Argo.hh"


def getContent(filename: str):
    ret = []
    flag = False
    with open(filename, "r") as f:
        for line in f:
            if line.startswith("// generator start here"):
                flag = True
                continue
            if line.startswith("// generator end here"):
                flag = False
                continue
            if flag:
                ret.append(line)

    return ret


def main():
    generated = []

    with open(base, "r") as f:
        for line in f:
            if line.startswith("// fetch"):
                filename = re.search(r"\{\s(.*?)\s\}", line).group(1)
                print("expanding", filename)
                generated += getContent("../" + filename)
            else:
                generated.append(line)

    # delete export keyword
    generated = [line.replace("export ", "") for line in generated]

    with open("../include/Argo.hh", "w+") as f:
        f.write("".join(generated))


if __name__ == "__main__":
    main()
