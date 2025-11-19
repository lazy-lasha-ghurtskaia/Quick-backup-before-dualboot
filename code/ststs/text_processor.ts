//part1

function TrimText(text: string): string {
	let trimmed: string = "";
	for (let i=0; i<text.length; i++){
		if (! (text[i] == " ") ){
			trimmed += text[i];
		}
	}
	return trimmed;
}

function ToUpper(text: string): string {
	let uppercased: string = "";
	for (let i = 0; i < text.length; i++){
		uppercased += text[i].toUpperCase();
	}
	return uppercased;
}

let bad_words: string[] = ["banana", "mango", "apple"];
function is_bad(text: string): boolean {
	for (let i = 0; i< bad_words.length; i++){
		if (text == bad_words[i])
			return true;
	}
	return false;
}

function censor(text: string): string {
	let censored: string = "";
	let start: number = 0;
	for (let i = 0; i<=text.length; i++){
		if (i == text.length || text[i] == " "){
			let word = text.substring(start,i);
			if (is_bad(word)){
				for (let j = 0; j< word.length; j++){
					censored += "*";
				}
			} else {
				censored += word;
			}

			if (i < text.length)
				censored += " ";
			start = i+1;
		}
	}
	return censored;
}

function Prefix(text: string, prefix: string): string {
	return prefix + text;
}

console.log("Part 1:");
console.log("Before trim:   Hello World  ");
process.stdout.write("After trim:");
console.log(TrimText("   Hello World  "));
console.log();

console.log("Before uppercase: Hello World");
process.stdout.write("After uppercase: ");
console.log(ToUpper("Hello World"));
console.log();

console.log("Before censoring: banana World");
process.stdout.write("After censoring: ");
console.log(censor("banana World"));
console.log();

console.log("Before prefixing: World");
process.stdout.write("After prefixing: ");
console.log(Prefix(" world", "Hello"));
console.log();




//part 2

function Prefix_wrap(text: string): string {
	return Prefix(text, text);
}

let test_text = "   hello banana   ";
const operations = [TrimText, ToUpper, censor, Prefix_wrap];
console.log("Part 2:");
operations.forEach(op => {console.log(op(test_text))})
//Prefix function inherently needs two parameters
//but for completeness i wrote a wrapper that prefixes 
//the string to itself
console.log();



// part 3

function ProcessText(operation: Array<(s: string) => string>, s: string): string{
	let computed: string = s;
	for (const op of operation){
		computed = op(computed);
	}
	return computed;
}
console.log("Part 3");
console.log(ProcessText([TrimText, ToUpper], "   hello banana   "));
console.log();



//part 4

//copied over my censorer code if you don't mind
function CreateCensor(bad_ones: string[]): (string:string) => string {
	return function (text:string): string {
		let censored: string = "";
		let start: number = 0;
		let bad_word: boolean = false;

		for (let i = 0; i<=text.length; i++){
			if (text[i] == " " || i == text.length){
				bad_word = false;
				let word = text.substring(start,i);

				for (let i = 0; i< bad_ones.length; i++){
					if (word == bad_ones[i])
						bad_word = true;
				}

				if (bad_word){
					for (let j = 0; j< word.length; j++){
						censored += "*";
					}
				} else {
					censored += word;
				}

				if (i < text.length)
					censored += " ";
				start = i+1;
			}
		}
		return censored;
	}
}
function CreatePrefixer(prefix: string): (string:string) => string {
	return function (s:string): string {
		return prefix + s;
	}
}
console.log("Part 4");
const censorer = CreateCensor(["bazil", "spinach"]);
console.log(censorer("bazil is better then spinach"));
const batteryWarning = CreatePrefixer("Battery Low!: ");
console.log(batteryWarning("connect to the charger please"));
console.log();



// part 5
// Can't make it in time
